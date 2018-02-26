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
  if ((error = mparray_init (&points_f, N)) != PRIO_OKAY)
    return error;
  if ((error = mparray_init (&poly_f, N)) != PRIO_OKAY)
    return error;

  // Set constant term f(0) to random
  if ((error = rand_int (&points_f.data[0], mod)) != PRIO_OKAY)
    return error;
  if ((error = mp_copy (&points_f.data[0], const_term)) != PRIO_OKAY)
    return error;

  // Set other values of f(x)
  for (int i=1; i<n; i++) {
    mp_copy (&data_in->data[i-1], &points_f.data[i]);
  }

  // Interpolate through the Nth roots of unity
  if ((error = fft(&poly_f, &points_f, cfg, true)) != PRIO_OKAY)
    return error;

  // Evaluate at all 2N-th roots of unity. 
  // To do so, first resize the eval arrays and fill upper
  // values with zeros.
  if ((error = mparray_resize (&poly_f, 2*N)) != PRIO_OKAY)
    return error;
  if ((error = mparray_resize (evals_out, 2*N)) != PRIO_OKAY)
    return error;
  
  // Evaluate at the 2N-th roots of unity
  if ((error = fft(evals_out, &poly_f, cfg, false)) != PRIO_OKAY)
    return error;

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

  if ((error = mparray_dup (&points_g, points_f) != PRIO_OKAY))
    return error;
  for (int i=0; i<points_f->len; i++) {
    // For each input value x_i, we compute x_i * (x_i-1).
    //    f(i) = x_i
    //    g(i) = x_i - 1
    mp_sub_d (&points_g.data[i], 1, &points_g.data[i]);
    mp_mod (&points_g.data[i], mod, &points_g.data[i]);
  }

  mp_int f0, g0;
  if (mp_init (&f0) != MP_OKAY)
    return PRIO_ERROR;
  if (mp_init (&g0) != MP_OKAY)
    return PRIO_ERROR;

  struct mparray evals_f_2N, evals_g_2N;
  if ((error = mparray_init (&evals_f_2N, 0) != PRIO_OKAY))
    return error;
  if ((error = mparray_init (&evals_g_2N, 0) != PRIO_OKAY))
    return error;

  if ((error = data_polynomial_evals(cfg, points_f, 
          &evals_f_2N, &f0)) != PRIO_OKAY)
    return error;
  if ((error = data_polynomial_evals(cfg, &points_g, 
          &evals_g_2N, &g0)) != PRIO_OKAY)
    return error;

  if (mp_mulmod (&f0, &g0, mod, &f0) != MP_OKAY)
    return PRIO_ERROR;

  // Must send to each server a share of the point
  //    h(0) = f(0)*g(0)
  if ((error = share_int (cfg, &f0, &pA->h0_share, &pB->h0_share)) != PRIO_OKAY)
    return error;

  const int lenN = (evals_f_2N.len/2) - 1;
  if ((error = mparray_init (&pA->h_coeffs, lenN)) != PRIO_OKAY)
    return error;
  if ((error = mparray_init (&pB->h_coeffs, lenN)) != PRIO_OKAY)
    return error;

  // We need to send to the servers the evaluations of
  //   f(r) * g(r)
  // for all 2N-th roots of unity r that are not also
  // N-th roots of unity.
  int j = 0;
  for (int i = 1; i < evals_f_2N.len - 1; i += 2) {
    if (mp_mulmod (&evals_f_2N.data[i], &evals_g_2N.data[i], 
          mod, &f0) != MP_OKAY)
      return PRIO_ERROR;
    if ((error = share_int (cfg, &f0, 
            &pA->h_coeffs.data[j], &pB->h_coeffs.data[j])) != PRIO_OKAY)
      return error;
    j++;
  }

  mparray_clear (&evals_f_2N);
  mparray_clear (&evals_g_2N);
  mparray_clear (&points_g);
  mp_clear (&f0);
  mp_clear (&g0);
  return PRIO_OKAY;
}


int 
PrioPacketClient_new (const_PrioConfig cfg, const bool *data_in,
    PrioPacketClient *ptrA, PrioPacketClient *ptrB)
{
  int error;

  if (!data_in || !ptrA || !ptrB)
    return PRIO_ERROR; 

  *ptrA = malloc (sizeof (**ptrA));
  if (!*ptrA) return PRIO_ERROR;
  *ptrB = malloc (sizeof (**ptrB));
  if (!*ptrB) return PRIO_ERROR;

  PrioPacketClient pA = *ptrA;
  PrioPacketClient pB = *ptrB;

  if ((error = triple_new (&pA->triple)) != PRIO_OKAY)
    return error;
  if ((error = triple_new (&pB->triple)) != PRIO_OKAY)
    return error;

  if (mp_init (&pA->h0_share) != MP_OKAY)
    return PRIO_ERROR;
  if (mp_init (&pB->h0_share) != MP_OKAY)
    return PRIO_ERROR;

  if ((error = triple_rand (cfg, &pA->triple, &pB->triple)) != PRIO_OKAY)
    return error;

  struct mparray client_data;
  if ((error = mparray_init_bool (&client_data, 
          cfg->num_data_fields, data_in)) != PRIO_OKAY)
    return error;

  if ((error = mparray_init_share (&pA->data_shares, 
          &pB->data_shares, &client_data, cfg)) != PRIO_OKAY)
    return error;

  if ((error = share_polynomials (cfg, &client_data, pA, pB) != PRIO_OKAY))
    return error;

  mparray_clear (&client_data);

  return PRIO_OKAY;
}

void
PrioPacketClient_clear (PrioPacketClient p)
{
  mparray_clear (&p->h_coeffs);
  mparray_clear (&p->data_shares);
  triple_clear (&p->triple);
  mp_clear (&p->h0_share);
  free (p);
}

