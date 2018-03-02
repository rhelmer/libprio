/*
 * Copyright (c) 2018, Henry Corrigan-Gibbs
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <mprio.h>

#include "libmpi/mpi.h"
#include "client.h"
#include "fft.h"
#include "mparray.h"
#include "server.h"
#include "util.h"

PrioServer 
PrioServer_new (const_PrioConfig cfg, int server_idx)
{
  PrioServer s = malloc (sizeof (*s));
  if (!s) return NULL;
  s->cfg = cfg;
  s->idx = server_idx;
  s->data_shares = MPArray_new (s->cfg->num_data_fields);
  if (!s->data_shares) { 
    free (s);
    return NULL;
  }

  return s;
}

void 
PrioServer_clear (PrioServer s)
{
  if (!s) return;

  MPArray_clear (s->data_shares);
  free(s);
}

SECStatus
PrioServer_aggregate (PrioServer s, const_PrioPacketClient p)
{
  return MPArray_addmod (s->data_shares, p->data_shares, &s->cfg->modulus);  
}

PrioTotalShare 
PrioTotalShare_new (const_PrioServer s)
{
  PrioTotalShare t = malloc (sizeof (*t));
  if (!t) return NULL;

  t->data_shares = MPArray_dup (s->data_shares);
  if (!t->data_shares) {
    free (t);
    return NULL;
  }

  return t;
}

SECStatus
PrioTotalShare_final (const_PrioConfig cfg, 
    unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB)
{
  if (tA->data_shares->len != cfg->num_data_fields)
    return SECFailure;
  if (tA->data_shares->len != tB->data_shares->len)
    return SECFailure;

  SECStatus rv = SECSuccess;

  mp_int tmp;
  MP_DIGITS (&tmp) = NULL;
  MP_CHECKC (mp_init (&tmp));

  for (int i=0; i<cfg->num_data_fields; i++) {
    MP_CHECKC (mp_addmod(&tA->data_shares->data[i], &tB->data_shares->data[i], 
          &cfg->modulus, &tmp));

    output[i] = tmp.dp[0];
  }

cleanup:
  mp_clear (&tmp);
  return rv;
}

void 
PrioTotalShare_clear (PrioTotalShare t)
{
  if (!t) return;
  MPArray_clear (t->data_shares);
  free (t);
}

static SECStatus
eval_poly (mp_int *value, const_MPArray coeffs, const mp_int *eval_at, 
    const_PrioConfig cfg)
{ 
  SECStatus rv = SECSuccess;
  const int n = coeffs->len;
  mp_int tmp;
  MP_DIGITS (&tmp) = NULL;
  MP_CHECKC (mp_init (&tmp)); 

  mp_set (value, 0);
  for (int i=0; i<n; i++) {
    MP_CHECKC (mp_exptmod_d (eval_at, i, &cfg->modulus, &tmp));
    MP_CHECKC (mp_mulmod (&tmp, &coeffs->data[i], &cfg->modulus, &tmp));
    MP_CHECKC (mp_addmod (&tmp, value, &cfg->modulus, value));
  }

cleanup:
  mp_clear (&tmp);
  return rv;
}

/*
 * Interpolate the polynomial through the points specified
 * by `poly_points` and evaluate this polynomial at the point
 * `eval_at`. Return the result as `value`.
 */
static SECStatus
interp_evaluate (mp_int *value, const_MPArray poly_points, 
    const mp_int *eval_at, const_PrioConfig cfg)
{
  SECStatus rv;
  MPArray coeffs = NULL;
  const int N = poly_points->len;
  mp_int roots[N];
  
  P_CHECKA (coeffs = MPArray_new (N));
  P_CHECKC (fft_get_roots (roots, N, cfg, false));

  // Interpolate polynomial through roots of unity
  P_CHECKC (fft (coeffs, poly_points, cfg, true)) 
  P_CHECKC (eval_poly (value, coeffs, eval_at, cfg));

cleanup:
  MPArray_clear (coeffs);
  return rv;
}

/*
 * Build shares of the polynomials f, g, and h used in the Prio verification
 * routine and evalute these polynomials at a random point determined
 * by the shared secret. Store the evaluations in the verifier object.
 */
static SECStatus
compute_shares (PrioVerifier v, const ServerSharedSecret secret, 
    const_PrioPacketClient p)
{
  SECStatus rv;
  const int n = v->cfg->num_data_fields + 1;
  const int N = next_power_of_two (n);
  mp_int eval_at;
  MP_DIGITS (&eval_at) = NULL;

  MPArray points_f = NULL;
  MPArray points_g = NULL;
  MPArray points_h = NULL;

  MP_CHECKC (mp_init (&eval_at)); 
  P_CHECKA (points_f = MPArray_new (N));
  P_CHECKA (points_g = MPArray_new (N));
  P_CHECKA (points_h = MPArray_new (2*N));

  // Use secret to generate random point
  MP_CHECKC (mp_read_unsigned_octets (&eval_at, &secret[0], SOUNDNESS_PARAM));
 
  // Reduce value into the field we're using. This 
  // doesn't yield exactly a uniformly random point,
  // but for values this large, it will be close
  // enough.
  MP_CHECKC (mp_mod (&eval_at, &v->cfg->modulus, &eval_at));

  // Client sends us the values of f(0) and g(0)
  MP_CHECKC (mp_copy(&p->f0_share, &points_f->data[0]));
  MP_CHECKC (mp_copy(&p->g0_share, &points_g->data[0]));
  MP_CHECKC (mp_copy(&p->h0_share, &points_h->data[0])); 

  for (int i=1; i<n; i++) {
    // [f](i) = i-th data share
    MP_CHECKC (mp_copy(&p->data_shares->data[i-1], &points_f->data[i]));

    // [g](i) = i-th data share minus 1
    // Only need to shift the share for 0-th server
    MP_CHECKC (mp_copy(&points_f->data[i], &points_g->data[i]));
    if (!v->idx) {
      MP_CHECKC (mp_sub_d(&points_g->data[i], 1, &points_g->data[i]));
      MP_CHECKC (mp_mod(&points_g->data[i], &v->cfg->modulus, &points_g->data[i]));
    }
  }

  int j = 0;
  for (int i=1; i<2*N; i+=2) {
    MP_CHECKC (mp_copy(&p->h_points->data[j++], &points_h->data[i]));
  }

  P_CHECKC (interp_evaluate (&v->share_fR, points_f, &eval_at, v->cfg));
  P_CHECKC (interp_evaluate (&v->share_gR, points_g, &eval_at, v->cfg));
  P_CHECKC (interp_evaluate (&v->share_hR, points_h, &eval_at, v->cfg));

cleanup:
  MPArray_clear (points_f);
  MPArray_clear (points_g);
  MPArray_clear (points_h);
  mp_clear (&eval_at);
  return rv;
}

PrioVerifier PrioVerifier_new (PrioServer s, const_PrioPacketClient p, 
    const ServerSharedSecret secret)
{
  SECStatus rv;
  PrioVerifier v = malloc (sizeof *v);
  if (!v) return NULL;
  v->cfg = s->cfg;
  v->idx = s->idx;
  v->clientp = p;

  MP_DIGITS (&v->share_fR) = NULL;
  MP_DIGITS (&v->share_gR) = NULL;
  MP_DIGITS (&v->share_hR) = NULL;

  MP_CHECKC (mp_init (&v->share_fR));
  MP_CHECKC (mp_init (&v->share_gR));
  MP_CHECKC (mp_init (&v->share_hR)); 

  // TODO: This can be done much faster by using the combined
  // interpolate-and-evaluate optimization described in the 
  // Prio paper.
  //
  // Compute share of f(r), g(r), h(r)
  P_CHECKC (compute_shares (v, secret, p)); 

cleanup:
  if (rv != SECSuccess) {
    PrioVerifier_clear (v);
    return NULL;
  }

  return v;
}


void PrioVerifier_clear (PrioVerifier v)
{
  if (v == NULL) return;
  mp_clear (&v->share_fR);
  mp_clear (&v->share_gR);
  mp_clear (&v->share_hR);
  free (v);
}

PrioPacketVerify1 
PrioVerifier_packet1 (const_PrioVerifier v)
{
  // See the Prio paper for details on how this works.
  // Appendix C descrives the MPC protocol used here.

  SECStatus rv = SECSuccess;
  PrioPacketVerify1 p = malloc (sizeof *p);
  if (!p) return NULL;

  MP_DIGITS (&p->share_d) = NULL;
  MP_DIGITS (&p->share_e) = NULL;

  MP_CHECKC (mp_init (&p->share_d));
  MP_CHECKC (mp_init (&p->share_e));

  // Compute corrections.
  //   [d] = [f(r)] - [a]
  MP_CHECKC (mp_sub (&v->share_fR, &v->clientp->triple->a, &p->share_d));
  MP_CHECKC (mp_mod (&p->share_d, &v->cfg->modulus, &p->share_d));

  //   [e] = [g(r)] - [b]
  MP_CHECKC (mp_sub (&v->share_gR, &v->clientp->triple->b, &p->share_e));
  MP_CHECKC (mp_mod (&p->share_e, &v->cfg->modulus, &p->share_e));

cleanup:
  if (rv != SECSuccess) {
    PrioPacketVerify1_clear (p);
    return NULL;
  }

  return p;
}

void 
PrioPacketVerify1_clear (PrioPacketVerify1 p)
{
  if (!p) return;
  mp_clear (&p->share_d);
  mp_clear (&p->share_e);
  free (p);
}

PrioPacketVerify2 
PrioVerifier_packet2 (const_PrioVerifier v,
    const_PrioPacketVerify1 pA, const_PrioPacketVerify1 pB)
{
  SECStatus rv = SECSuccess;
  PrioPacketVerify2 p = malloc (sizeof *p);
  if (!p) return NULL;

  mp_int d, e, tmp;
  MP_DIGITS (&p->share_out) = NULL;
  MP_DIGITS (&d) = NULL;
  MP_DIGITS (&e) = NULL;
  MP_DIGITS (&tmp) = NULL;

  MP_CHECKC (mp_init (&p->share_out));
  MP_CHECKC (mp_init (&d));
  MP_CHECKC (mp_init (&e));
  MP_CHECKC (mp_init (&tmp));

  // Compute share of f(r)*g(r)
  //    [f(r)*g(r)] = [d*e/2] + d[b] + e[a] + [c]
 
 // Compute d 
  MP_CHECKC (mp_addmod (&pA->share_d, &pB->share_d, &v->cfg->modulus, &d));
  // Compute e
  MP_CHECKC (mp_addmod (&pA->share_e, &pB->share_e, &v->cfg->modulus, &e));

  // Compute d*e
  MP_CHECKC (mp_mulmod (&d, &e, &v->cfg->modulus, &p->share_out));
  // out = d*e/2
  MP_CHECKC (mp_mulmod (&p->share_out, &v->cfg->inv2, 
        &v->cfg->modulus, &p->share_out));

  // Compute d[b] 
  MP_CHECKC (mp_mulmod (&d, &v->clientp->triple->b, 
        &v->cfg->modulus, &tmp));
  // out = d*e/2 + d[b] 
  MP_CHECKC (mp_addmod (&p->share_out, &tmp, &v->cfg->modulus, &p->share_out));

  // Compute e[a] 
  MP_CHECKC (mp_mulmod (&e, &v->clientp->triple->a, &v->cfg->modulus, &tmp));
  // out = d*e/2 + d[b] + e[a]
  MP_CHECKC (mp_addmod (&p->share_out, &tmp, &v->cfg->modulus, &p->share_out));

  // out = d*e/2 + d[b] + e[a] + [c]
  MP_CHECKC (mp_addmod (&p->share_out, &v->clientp->triple->c, 
        &v->cfg->modulus, &p->share_out));

  // We want to compute f(r)*g(r) - h(r),
  // so subtract off [h(r)]:
  //    out = d*e/2 + d[b] + e[a] + [c] - [h(r)]
  MP_CHECKC (mp_sub (&p->share_out, &v->share_hR, &p->share_out));
  MP_CHECKC (mp_mod (&p->share_out, &v->cfg->modulus, &p->share_out));

cleanup:
  if (rv != SECSuccess) {
    PrioPacketVerify2_clear (p); 
  }
  mp_clear (&d);
  mp_clear (&e);
  mp_clear (&tmp);
  return p;
}

void 
PrioPacketVerify2_clear (PrioPacketVerify2 p)
{
  if (!p) return; 
  mp_clear (&p->share_out);
  free (p);
}

int 
PrioVerifier_isValid (const_PrioVerifier v,
    const_PrioPacketVerify2 pA, const_PrioPacketVerify2 pB)
{
  SECStatus rv = SECSuccess;
  mp_int res;
  MP_DIGITS (&res) = NULL;
  MP_CHECKC (mp_init (&res));

  // Add up the shares of the output wire value and 
  // ensure that the sum is equal to zero, which indicates
  // that
  //      f(r) * g(r) == h(r).
  MP_CHECKC (mp_addmod (&pA->share_out, &pB->share_out,
        &v->cfg->modulus, &res));

  rv = (mp_cmp_d (&res, 0) == 0) ? SECSuccess : SECFailure;

cleanup:
  mp_clear (&res);
  return rv;
}

