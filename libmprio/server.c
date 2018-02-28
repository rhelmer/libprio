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

#include <stdio.h>
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
 
  s->data_shares = MPArray_init (s->cfg->num_data_fields);
  if (!s->data_shares) return NULL;

  return s;
}

void 
PrioServer_clear (PrioServer s)
{
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
  if (!t->data_shares) return NULL;

  return t;
}

SECStatus
PrioTotalShare_final (const_PrioConfig cfg, 
    unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB)
{
  mp_int tmp;
  MP_CHECK (mp_init (&tmp));
  if (tA->data_shares->len != cfg->num_data_fields)
    return SECFailure;
  if (tA->data_shares->len != tB->data_shares->len)
    return SECFailure;

  for (int i=0; i<cfg->num_data_fields; i++) {
    MP_CHECK (mp_addmod(&tA->data_shares->data[i], &tB->data_shares->data[i], 
          &cfg->modulus, &tmp));

    output[i] = tmp.dp[0];
  }

  mp_clear (&tmp);
  return SECSuccess;
}

void 
PrioTotalShare_clear (PrioTotalShare t)
{
  MPArray_clear (t->data_shares);
  free (t);
}

static SECStatus
eval_poly (mp_int *value, const_MPArray coeffs, const mp_int *eval_at, const_PrioConfig cfg)
{ 
  const int n = coeffs->len;
  mp_int tmp;
  MP_CHECK (mp_init (&tmp)); 

  mp_set (value, 0);
  for (int i=0; i<n; i++) {
    MP_CHECK (mp_exptmod_d (eval_at, i, &cfg->modulus, &tmp));
    MP_CHECK (mp_mulmod (&tmp, &coeffs->data[i], &cfg->modulus, &tmp));
    MP_CHECK (mp_addmod (&tmp, value, &cfg->modulus, value));
  }

  mp_clear (&tmp);
  return SECSuccess;
}

static SECStatus
interp_evaluate (mp_int *value, const_MPArray poly_points, 
    const mp_int *eval_at, const_PrioConfig cfg)
{
  SECStatus rv;
  const int N = poly_points->len;
  MPArray coeffs = MPArray_init (N);
  if (!coeffs) return SECFailure;

  mp_int roots[N];
  fft_get_roots (roots, N, cfg, false);

  // Interpolate polynomial through roots of unity
  P_CHECK (fft (coeffs, poly_points, cfg, true)) 
  P_CHECK (eval_poly (value, coeffs, eval_at, cfg));

  MPArray_clear (coeffs);
  return SECSuccess;
}

static SECStatus
compute_shares (PrioVerifier v, ServerSharedSecret secret, const_PrioPacketClient p)
{
  const int n = v->cfg->num_data_fields + 1;
  const int N = next_power_of_two (n);
  mp_int eval_at;
  MP_CHECK (mp_init (&eval_at)); 

  // Use secret to generate random point
  MP_CHECK (mp_read_unsigned_octets (&eval_at, &secret[0], SOUNDNESS_PARAM));
 
  // Reduce value into the field we're using. This 
  // doesn't yield exactly a uniformly random point,
  // but for values this large, it will be close
  // enough.
  MP_CHECK (mp_mod (&eval_at, &v->cfg->modulus, &eval_at));

  MPArray points_f = MPArray_init (N);
  if (!points_f) return SECFailure;
  MPArray points_g = MPArray_init (N);
  if (!points_g) return SECFailure;
  MPArray points_h = MPArray_init (2*N);
  if (!points_h) return SECFailure;

  // Client sends us the values of f(0) and g(0)
  MP_CHECK (mp_copy(&p->f0_share, &points_f->data[0]));
  MP_CHECK (mp_copy(&p->g0_share, &points_g->data[0]));
  MP_CHECK (mp_copy(&p->h0_share, &points_h->data[0])); 

  for (int i=1; i<n; i++) {
    // [f](i) = i-th data share
    MP_CHECK (mp_copy(&p->data_shares->data[i-1], &points_f->data[i]));

    // [g](i) = i-th data share minus 1
    // Only need to shift the share for 0-th server
    MP_CHECK (mp_copy(&points_f->data[i], &points_g->data[i]));
    if (!v->idx) {
      MP_CHECK (mp_sub_d(&points_g->data[i], 1, &points_g->data[i]));
      MP_CHECK (mp_mod(&points_g->data[i], &v->cfg->modulus, &points_g->data[i]));
    }
  }

  int j = 0;
  for (int i=1; i<2*N; i+=2) {
    MP_CHECK (mp_copy(&p->h_points->data[j++], &points_h->data[i]));
  }

  SECStatus rv;
  P_CHECK (interp_evaluate (&v->share_fR, points_f, &eval_at, v->cfg));
  P_CHECK (interp_evaluate (&v->share_gR, points_g, &eval_at, v->cfg));
  P_CHECK (interp_evaluate (&v->share_hR, points_h, &eval_at, v->cfg));

  MPArray_clear (points_f);
  MPArray_clear (points_g);
  MPArray_clear (points_h);
  mp_clear (&eval_at);
  return SECSuccess;
}

PrioVerifier PrioVerifier_new (PrioServer s, const_PrioPacketClient p, 
    ServerSharedSecret secret)
{
  PrioVerifier v = malloc (sizeof *v);
  if (!v) return NULL;
  v->cfg = s->cfg;
  v->idx = s->idx;
  v->clientp = p;

  MP_CHECKN (mp_init (&v->share_fR));
  MP_CHECKN (mp_init (&v->share_gR));
  MP_CHECKN (mp_init (&v->share_hR)); 

  // TODO: This can be done much faster by using the combined
  // interpolate-and-evaluate optimization described in the 
  // Prio paper.
  //
  // Compute share of f(r), g(r), h(r)

  P_CHECKN (compute_shares (v, secret, p));

  return v;
}


void PrioVerifier_clear (PrioVerifier v)
{
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

  PrioPacketVerify1 p = malloc (sizeof *p);
  if (!p) return NULL;

  MP_CHECKN (mp_init (&p->share_d));
  MP_CHECKN (mp_init (&p->share_e));

  // Compute corrections.
  //   [d] = [f(r)] - [a]
  MP_CHECKN (mp_sub (&v->share_fR, &v->clientp->triple.a, &p->share_d));
  MP_CHECKN (mp_mod (&p->share_d, &v->cfg->modulus, &p->share_d));

  //   [e] = [g(r)] - [b]
  MP_CHECKN (mp_sub (&v->share_gR, &v->clientp->triple.b, &p->share_e));
  MP_CHECKN (mp_mod (&p->share_e, &v->cfg->modulus, &p->share_e));

  return p;
}

void 
PrioPacketVerify1_clear (PrioPacketVerify1 p)
{
  mp_clear (&p->share_d);
  mp_clear (&p->share_e);
  free (p);
}

PrioPacketVerify2 
PrioVerifier_packet2 (const_PrioVerifier v,
    const_PrioPacketVerify1 pA, const_PrioPacketVerify1 pB)
{
  PrioPacketVerify2 p = malloc (sizeof *p);
  if (!p) return NULL;

  mp_int d, e, tmp;
  MP_CHECKN (mp_init (&p->share_out));
  MP_CHECKN (mp_init (&d));
  MP_CHECKN (mp_init (&e));
  MP_CHECKN (mp_init (&tmp));

  // Compute share of f(r)*g(r)
  //    [f(r)*g(r)] = [d*e/2] + d[b] + e[a] + [c]
 
 // Compute d 
  MP_CHECKN (mp_addmod (&pA->share_d, &pB->share_d, &v->cfg->modulus, &d));
  // Compute e
  MP_CHECKN (mp_addmod (&pA->share_e, &pB->share_e, &v->cfg->modulus, &e));

  // Compute d*e
  MP_CHECKN (mp_mulmod (&d, &e, &v->cfg->modulus, &p->share_out));
  // out = d*e/2
  MP_CHECKN (mp_mulmod (&p->share_out, &v->cfg->inv2, 
        &v->cfg->modulus, &p->share_out));

  // Compute d[b] 
  MP_CHECKN (mp_mulmod (&d, &v->clientp->triple.b, 
        &v->cfg->modulus, &tmp));
  // out = d*e/2 + d[b] 
  MP_CHECKN (mp_addmod (&p->share_out, &tmp, &v->cfg->modulus, &p->share_out));

  // Compute e[a] 
  MP_CHECKN (mp_mulmod (&e, &v->clientp->triple.a, &v->cfg->modulus, &tmp));
  // out = d*e/2 + d[b] + e[a]
  MP_CHECKN (mp_addmod (&p->share_out, &tmp, &v->cfg->modulus, &p->share_out));

  // out = d*e/2 + d[b] + e[a] + [c]
  MP_CHECKN (mp_addmod (&p->share_out, &v->clientp->triple.c, 
        &v->cfg->modulus, &p->share_out));

  // We want to compute f(r)*g(r) - h(r),
  // so subtract off [h(r)]:
  //    out = d*e/2 + d[b] + e[a] + [c] - [h(r)]
  MP_CHECKN (mp_sub (&p->share_out, &v->share_hR, &p->share_out));
  MP_CHECKN (mp_mod (&p->share_out, &v->cfg->modulus, &p->share_out));

  mp_clear (&d);
  mp_clear (&e);
  mp_clear (&tmp);
  return p;
}

void 
PrioPacketVerify2_clear (PrioPacketVerify2 p)
{
  mp_clear (&p->share_out);
  free (p);
}

int 
PrioVerifier_isValid (const_PrioVerifier v,
    const_PrioPacketVerify2 pA, const_PrioPacketVerify2 pB)
{
  mp_int res;
  MP_CHECK (mp_init (&res));
  
  MP_CHECK (mp_addmod (&pA->share_out, &pB->share_out,
        &v->cfg->modulus, &res));

  bool good = (mp_cmp_d (&res, 0) == 0);

  mp_clear (&res);
  return good ? SECSuccess : SECFailure;
}

