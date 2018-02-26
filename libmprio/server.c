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
PrioServer_new (const_PrioConfig cfg)
{
  PrioServer s = malloc (sizeof (*s));
  if (!s) return NULL;
  s->cfg = cfg;
  
  if (mparray_init (&s->data_shares, s->cfg->num_data_fields) != PRIO_OKAY)
    return NULL;

  return s;
}

void 
PrioServer_clear (PrioServer s)
{
  mparray_clear (&s->data_shares);
  free(s);
}

int 
PrioServer_aggregate (PrioServer s, const_PrioPacketClient p)
{
  return mparray_addmod (&s->data_shares, &p->data_shares, &s->cfg->modulus);  
}

PrioTotalShare 
PrioTotalShare_new (const_PrioServer s)
{
  PrioTotalShare t = malloc (sizeof (*t));
  if (!t) return NULL;
  
  if (mparray_dup (&t->data_shares, &s->data_shares) != PRIO_OKAY)
    return NULL;

  return t;
}

int PrioTotalShare_final (const_PrioConfig cfg, 
    unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB)
{
  mp_int tmp;
  MP_CHECK (mp_init (&tmp));
  if (tA->data_shares.len != cfg->num_data_fields)
    return PRIO_ERROR;
  if (tA->data_shares.len != tB->data_shares.len)
    return PRIO_ERROR;

  for (int i=0; i<cfg->num_data_fields; i++) {
    MP_CHECK (mp_addmod(&tA->data_shares.data[i], &tB->data_shares.data[i], 
          &cfg->modulus, &tmp));

    output[i] = tmp.dp[0];
  }

  mp_clear (&tmp);
  return PRIO_OKAY;
}

void 
PrioTotalShare_clear (PrioTotalShare t)
{
  mparray_clear (&t->data_shares);
  free (t);
}

static int
eval_poly (mp_int *value, const struct mparray *coeffs, const mp_int *eval_at, const_PrioConfig cfg)
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
  return PRIO_OKAY;
}

static int
interp_evaluate (mp_int *value, const struct mparray *poly_points, 
    const mp_int *eval_at, const_PrioConfig cfg)
{
  int error;
  const int N = poly_points->len;
  struct mparray coeffs;
  if (mparray_init (&coeffs, N) != PRIO_OKAY)
    return PRIO_ERROR;

  // Interpolate polynomial through roots of unity
  if ((error = fft (&coeffs, poly_points, cfg, true)) != PRIO_OKAY)
    return error;

  if ((error = eval_poly (value, &coeffs, eval_at, cfg)) != PRIO_OKAY)
    return error;

  mparray_clear (&coeffs);
  return PRIO_OKAY;
}

static int
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
  
  struct mparray points_f, points_g, points_h;
  if (mparray_init (&points_f, N) != PRIO_OKAY)
    return PRIO_ERROR;
  if (mparray_init (&points_g, N) != PRIO_OKAY)
    return PRIO_ERROR;
  if (mparray_init (&points_h, 2*N - 1) != PRIO_OKAY)
    return PRIO_ERROR;

  // Client sends us the values of f(0) and g(0)
  MP_CHECK (mp_copy(&p->f0_share, &points_f.data[0]));
  MP_CHECK (mp_copy(&p->g0_share, &points_g.data[0]));
  MP_CHECK (mp_copy(&p->h0_share, &points_h.data[0])); 

  for (int i=1; i<n; i++) {
    // [f](i) = i-th data share
    MP_CHECK (mp_copy(&p->data_shares.data[i-1], &points_f.data[i]));

    // [g](i) = i-th data share minus 1
    MP_CHECK (mp_sub_d(&points_f.data[i], 1, &points_g.data[i]));
    MP_CHECK (mp_mod(&points_g.data[i], &v->cfg->modulus, &points_g.data[i]));
  }

  int j = 0;
  for (int i=1; i<2*N-1; i+=2) {
    MP_CHECK (mp_copy(&p->h_points.data[j++], &points_h.data[i]));
  }

 
  int error;
  P_CHECK (interp_evaluate (&v->share_fR, &points_f, &eval_at, v->cfg));
  P_CHECK (interp_evaluate (&v->share_gR, &points_g, &eval_at, v->cfg));
  P_CHECK (interp_evaluate (&v->share_hR, &points_h, &eval_at, v->cfg));

  mparray_clear (&points_f);
  mparray_clear (&points_g);
  mparray_clear (&points_h);
  mp_clear (&eval_at);
  return PRIO_OKAY;
}

PrioVerifier PrioVerifier_new (PrioServer s, const_PrioPacketClient p, 
    ServerSharedSecret secret)
{
  PrioVerifier v = malloc (sizeof *v);
  if (!v) return NULL;
  v->cfg = s->cfg;

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
  free (v);
}

/*
PrioPacketVerify1 PrioVerifier_packet1 (const_PrioVerifier v);
void PrioPacketVerify1_clear (PrioPacketVerify1 p);

PrioPacketVerify2 PrioVerifier_packet2 (const_PrioVerifier v,
    PrioPacketVerify1 pA, PrioPacketVerify1 pB);
int PrioVerifier_isValid (const_PrioVerifier v,
    PrioPacketVerify2 pA, PrioPacketVerify2 pB);
void PrioPacketVerify2_clear (PrioPacketVerify2 p);
*/
