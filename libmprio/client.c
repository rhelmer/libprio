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
#include "config.h"
#include "fft.h"
#include "rand.h"
#include "util.h"

// Let the points of data_in be [x1, x2, x3, ... ].
// We construct the polynomial f such that 
// (a)    f(0) = random,
// (b)    f(i) = x_i  for all i >= 1,
// (c)    degree(f)+1 is a power of two.
// We then evaluate f at the 2N-th roots of unity
// and we return these evaluations as `evals_out`
// and we return f(0) as `const_term`.
static int
data_polynomial_evals(const_PrioConfig cfg, const struct mparray *data_in,
    struct mparray *evals_out, mp_int *const_term)
{
  const mp_int *mod = &cfg->modulus;

  // Number of multiplication gates in the Valid() circuit.
  const int mul_gates = cfg->num_data_fields;

  // Little n is the number of points on the polynomials.
  // The constant term is randomized, so it's (mul_gates + 1).
  const int n = mul_gates + 1;
 
  // Big N is n rounded up to a power of two.
  const int N = next_power_of_two (n);

  int error;
  struct mparray points_f, poly_f;
  P_CHECK (mparray_init (&points_f, N)); 
  P_CHECK (mparray_init (&poly_f, N)); 

  // Set constant term f(0) to random
  P_CHECK (rand_int (&points_f.data[0], mod)); 
  P_CHECK (mp_copy (&points_f.data[0], const_term)); 

  // Set other values of f(x)
  for (int i=1; i<n; i++) {
    mp_copy (&data_in->data[i-1], &points_f.data[i]);
  }

  // Interpolate through the Nth roots of unity
  P_CHECK (fft(&poly_f, &points_f, cfg, true)); 

  // Evaluate at all 2N-th roots of unity. 
  // To do so, first resize the eval arrays and fill upper
  // values with zeros.
  P_CHECK (mparray_resize (&poly_f, 2*N)); 
  P_CHECK (mparray_resize (evals_out, 2*N)); 
  
  // Evaluate at the 2N-th roots of unity
  P_CHECK (fft(evals_out, &poly_f, cfg, false)); 

  mparray_clear (&points_f);
  mparray_clear (&poly_f);

  return PRIO_OKAY;
}


static int
share_polynomials (const_PrioConfig cfg, const struct mparray *data_in,
    PrioPacketClient pA, PrioPacketClient pB)
{
  int error;
  const mp_int *mod = &cfg->modulus;

  const struct mparray *points_f = data_in;
  struct mparray points_g;

  P_CHECK (mparray_dup (&points_g, points_f)); 
  for (int i=0; i<points_f->len; i++) {
    // For each input value x_i, we compute x_i * (x_i-1).
    //    f(i) = x_i
    //    g(i) = x_i - 1
    mp_sub_d (&points_g.data[i], 1, &points_g.data[i]);
    mp_mod (&points_g.data[i], mod, &points_g.data[i]);
  }

  mp_int f0, g0;
  MP_CHECK (mp_init (&f0)); 
  MP_CHECK (mp_init (&g0)); 

  struct mparray evals_f_2N, evals_g_2N;
  P_CHECK (mparray_init (&evals_f_2N, 0)); 
  P_CHECK (mparray_init (&evals_g_2N, 0)); 

  P_CHECK (data_polynomial_evals(cfg, points_f, &evals_f_2N, &f0));
  P_CHECK (data_polynomial_evals(cfg, &points_g, &evals_g_2N, &g0));

  // Must send to each server a share of the points
  //    f(0),   g(0),   and   h(0) = f(0)*g(0)
  P_CHECK (share_int (cfg, &f0, &pA->f0_share, &pB->f0_share)); 
  P_CHECK (share_int (cfg, &g0, &pA->g0_share, &pB->g0_share)); 

  MP_CHECK (mp_mulmod (&f0, &g0, mod, &f0));

  P_CHECK (share_int (cfg, &f0, &pA->h0_share, &pB->h0_share)); 

  const int lenN = (evals_f_2N.len/2);
  P_CHECK (mparray_init (&pA->h_points, lenN)); 
  P_CHECK (mparray_init (&pB->h_points, lenN)); 

  // We need to send to the servers the evaluations of
  //   f(r) * g(r)
  // for all 2N-th roots of unity r that are not also
  // N-th roots of unity.
  int j = 0;
  for (int i = 1; i < evals_f_2N.len; i += 2) {
    MP_CHECK (mp_mulmod (&evals_f_2N.data[i], &evals_g_2N.data[i], mod, &f0));
    P_CHECK (share_int (cfg, &f0, &pA->h_points.data[j], &pB->h_points.data[j])); 
    j++;
  }

  for (int i = 0; i < evals_f_2N.len; i += 2) {
    MP_CHECK (mp_mulmod (&evals_f_2N.data[i], &evals_g_2N.data[i], mod, &f0));
  }

  mparray_clear (&evals_f_2N);
  mparray_clear (&evals_g_2N);
  mparray_clear (&points_g);
  mp_clear (&f0);
  mp_clear (&g0);
  return PRIO_OKAY;
}

static int
init_objects(PrioPacketClient *ptr)
{
  if (!ptr)
    return PRIO_ERROR; 

  *ptr = malloc (sizeof (**ptr));
  if (!*ptr) return PRIO_ERROR;

  int error;
  PrioPacketClient p = *ptr;
  P_CHECK (triple_new (&p->triple));

  MP_CHECK (mp_init (&p->f0_share)); 
  MP_CHECK (mp_init (&p->g0_share));
  MP_CHECK (mp_init (&p->h0_share));

  return PRIO_OKAY;
}

int 
PrioPacketClient_new (const_PrioConfig cfg, const bool *data_in,
    PrioPacketClient *ptrA, PrioPacketClient *ptrB)
{
  int error;

  if (!data_in) return PRIO_ERROR; 

  P_CHECK (init_objects (ptrA)); 
  P_CHECK (init_objects (ptrB)); 

  PrioPacketClient pA = *ptrA;
  PrioPacketClient pB = *ptrB;

  P_CHECK (triple_rand (cfg, &pA->triple, &pB->triple)); 

  struct mparray client_data;
  P_CHECK (mparray_init_bool (&client_data, cfg->num_data_fields, data_in));
  P_CHECK (mparray_init_share (&pA->data_shares, &pB->data_shares, &client_data, cfg)); 
  P_CHECK (share_polynomials (cfg, &client_data, pA, pB)); 

  mparray_clear (&client_data);

  return PRIO_OKAY;
}

void
PrioPacketClient_clear (PrioPacketClient p)
{
  mparray_clear (&p->h_points);
  mparray_clear (&p->data_shares);
  triple_clear (&p->triple);
  mp_clear (&p->f0_share);
  mp_clear (&p->g0_share);
  mp_clear (&p->h0_share);
  free (p);
}

