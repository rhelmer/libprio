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


#include <mprio.h>

#include "libmpi/mpi.h"
#include "libmprio/config.h"
#include "libmprio/fft.h"
#include "mutest.h"

void 
mu_test__fft_one (void)
{
  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  mp_int a, b;
  mu_check (mp_init (&a) == MP_OKAY);
  mu_check (mp_init (&b) == MP_OKAY);

  const mp_int points_in[] = { a };
  mp_set (&a, 3);
  mp_int points_out[] = { b };
  mu_check (fft_interpolate(points_out, points_in, 1, cfg, false) == PRIO_OKAY);

  mu_check (mp_cmp_d(&a, 3) == 0);
  mu_check (mp_cmp_d(&b, 3) == 0);

  mp_clear (&a);
  mp_clear (&b);

  PrioConfig_clear (cfg);
}

void 
mu_test__fft_roots (void)
{
  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  mp_int roots[4];
  fft_get_roots (roots, 4, cfg, false);

  mp_int tmp;
  mp_init (&tmp);

  for (int i=0; i<4; i++) {
    mp_exptmod_d(&roots[i], 4, &cfg->modulus, &tmp);
    mu_check (mp_cmp_d( &tmp, 1) == 0);
  }

  mp_clear (&tmp);
  PrioConfig_clear (cfg);
}

void 
mu_test__fft_simple (void)
{
  const int nPoints = 4;

  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  mp_int points_in[nPoints];
  mp_int points_out[nPoints];
  mp_int roots[nPoints];
  for (int i=0; i<nPoints; i++) {
    mu_check (mp_init (&points_in[i]) == MP_OKAY);
    mu_check (mp_init (&points_out[i]) == MP_OKAY);
  }

  fft_get_roots (roots, nPoints, cfg, false);

  mp_set (&points_in[0], 3);
  mp_set (&points_in[1], 8);
  mp_set (&points_in[2], 7);
  mp_set (&points_in[3], 9);
  mu_check (fft_interpolate(points_out, points_in, nPoints, cfg, false) == PRIO_OKAY);

  mp_int should_be, tmp;
  mp_init (&should_be);
  mp_init (&tmp);

  for (int i=0; i<nPoints; i++) {
    mp_set (&should_be, 0);
    for (int j=0; j<nPoints; j++) {
      mu_check (mp_exptmod_d(&roots[i], j, &cfg->modulus, &tmp) == MP_OKAY);
      mu_check (mp_mulmod(&tmp, &points_in[j], &cfg->modulus, &tmp) == MP_OKAY);
      mu_check (mp_addmod(&should_be, &tmp, &cfg->modulus, &should_be) == MP_OKAY);
    }

    /*
    puts("Should be:");
    mp_print(&should_be, stdout);
    puts("");
    mp_print(&points_out[i], stdout);
    puts("");
    */
    mu_check (mp_cmp (&should_be, &points_out[i]) == 0);
  }

  mp_clear (&tmp);
  mp_clear (&should_be);
  for (int i=0; i<nPoints; i++) {
    mp_clear (&points_in[i]); 
    mp_clear (&points_out[i]);
  }
  PrioConfig_clear (cfg);
}

void 
mu_test__fft_invert (void)
{
  const int nPoints = 8;

  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  mp_int points_in[nPoints];
  mp_int points_out[nPoints];
  mp_int points_out2[nPoints];
  mp_int roots[nPoints];
  for (int i=0; i<nPoints; i++) {
    mu_check (mp_init (&points_in[i]) == MP_OKAY);
    mu_check (mp_init (&points_out[i]) == MP_OKAY);
    mu_check (mp_init (&points_out2[i]) == MP_OKAY);
  }

  fft_get_roots (roots, nPoints, cfg, false);

  mp_set (&points_in[0], 3);
  mp_set (&points_in[1], 8);
  mp_set (&points_in[2], 7);
  mp_set (&points_in[3], 9);
  mp_set (&points_in[4], 123);
  mp_set (&points_in[5], 123123987);
  mp_set (&points_in[6], 2);
  mp_set (&points_in[7], 0);
  mu_check (fft_interpolate(points_out, points_in, nPoints, cfg, false) == PRIO_OKAY);
  mu_check (fft_interpolate(points_out2, points_out, nPoints, cfg, true) == PRIO_OKAY);

  for (int i=0; i<nPoints; i++) {
    mu_check (mp_cmp (&points_out2[i], &points_in[i]) == 0);
  }

  for (int i=0; i<nPoints; i++) {
    mp_clear (&points_in[i]); 
    mp_clear (&points_out[i]);
    mp_clear (&points_out2[i]);
  }
  PrioConfig_clear (cfg);
}
