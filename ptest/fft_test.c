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
#include "libmprio/mparray.h"
#include "mutest.h"

void 
mu_test__fft_one (void)
{
  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  MPArray points_in = MPArray_init(1);
  MPArray points_out = MPArray_init(1);
  mu_ensure (points_in);
  mu_ensure (points_out);

  mp_set (&points_in->data[0], 3);
  mu_check (fft(points_out, points_in, cfg, false) == SECSuccess);

  mu_check (mp_cmp_d(&points_in->data[0], 3) == 0);
  mu_check (mp_cmp_d(&points_out->data[0], 3) == 0);

  MPArray_clear (points_in);
  MPArray_clear (points_out);

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

  MPArray points_in = MPArray_init (nPoints);
  MPArray points_out = MPArray_init (nPoints);
  mu_ensure (points_in);
  mu_ensure (points_out);

  mp_int roots[nPoints];
  fft_get_roots (roots, nPoints, cfg, false);

  mp_set (&points_in->data[0], 3);
  mp_set (&points_in->data[1], 8);
  mp_set (&points_in->data[2], 7);
  mp_set (&points_in->data[3], 9);
  mu_check (fft (points_out, points_in, cfg, false) == SECSuccess);

  mp_int should_be, tmp;
  mp_init (&should_be);
  mp_init (&tmp);

  for (int i=0; i<nPoints; i++) {
    mp_set (&should_be, 0);
    for (int j=0; j<nPoints; j++) {
      mu_check (mp_exptmod_d(&roots[i], j, &cfg->modulus, &tmp) == MP_OKAY);
      mu_check (mp_mulmod(&tmp, &points_in->data[j], &cfg->modulus, &tmp) == MP_OKAY);
      mu_check (mp_addmod(&should_be, &tmp, &cfg->modulus, &should_be) == MP_OKAY);
    }

    /*
    puts("Should be:");
    mp_print(&should_be, stdout);
    puts("");
    mp_print(&points_out[i], stdout);
    puts("");
    */
    mu_check (mp_cmp (&should_be, &points_out->data[i]) == 0);
  }

  mp_clear (&tmp);
  mp_clear (&should_be);
  MPArray_clear (points_in);
  MPArray_clear (points_out);
  PrioConfig_clear (cfg);
}

void 
mu_test__fft_invert (void)
{
  const int nPoints = 8;

  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  MPArray points_in = MPArray_init (nPoints);
  MPArray points_out = MPArray_init (nPoints);
  MPArray points_out2 = MPArray_init (nPoints);
  mu_ensure (points_in);
  mu_ensure (points_out);
  mu_ensure (points_out2);

  mp_int roots[nPoints];
  fft_get_roots (roots, nPoints, cfg, false);

  mp_set (&points_in->data[0], 3);
  mp_set (&points_in->data[1], 8);
  mp_set (&points_in->data[2], 7);
  mp_set (&points_in->data[3], 9);
  mp_set (&points_in->data[4], 123);
  mp_set (&points_in->data[5], 123123987);
  mp_set (&points_in->data[6], 2);
  mp_set (&points_in->data[7], 0);
  mu_check (fft(points_out, points_in, cfg, false) == SECSuccess);
  mu_check (fft(points_out2, points_out, cfg, true) == SECSuccess);

  for (int i=0; i<nPoints; i++) {
    mu_check (mp_cmp (&points_out2->data[i], &points_in->data[i]) == 0);
  }

  MPArray_clear (points_in);
  MPArray_clear (points_out);
  MPArray_clear (points_out2);
  PrioConfig_clear (cfg);
}
