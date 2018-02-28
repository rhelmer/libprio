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
#include "libmprio/util.h"
#include "mutest.h"

void 
mu_test__fft_one (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  MPArray points_in = NULL;
  MPArray points_out = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (points_in = MPArray_init(1));
  P_CHECKA (points_out = MPArray_init(1));

  mp_set (&points_in->data[0], 3);
  mu_check (fft(points_out, points_in, cfg, false) == SECSuccess);

  mu_check (mp_cmp_d(&points_in->data[0], 3) == 0);
  mu_check (mp_cmp_d(&points_out->data[0], 3) == 0);

cleanup:
  mu_check (rv == SECSuccess);
  MPArray_clear (points_in);
  MPArray_clear (points_out);

  PrioConfig_clear (cfg);
}

void 
mu_test__fft_roots (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  mp_int tmp;
  MP_DIGITS (&tmp) = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());
  MP_CHECKC (mp_init (&tmp));

  mp_int roots[4];
  fft_get_roots (roots, 4, cfg, false);

  for (int i=0; i<4; i++) {
    mp_exptmod_d(&roots[i], 4, &cfg->modulus, &tmp);
    mu_check (mp_cmp_d( &tmp, 1) == 0);
  }

cleanup:
  mu_check (rv == SECSuccess);
  mp_clear (&tmp);
  PrioConfig_clear (cfg);
}

void 
mu_test__fft_simple (void)
{
  SECStatus rv = SECSuccess;
  const int nPoints = 4;

  PrioConfig cfg = NULL;
  MPArray points_in = NULL;
  MPArray points_out = NULL;

  mp_int should_be, tmp;
  mp_int roots[nPoints];
  MP_DIGITS (&should_be) = NULL;
  MP_DIGITS (&tmp) = NULL;
  for (int i=0; i<nPoints; i++) {
    MP_DIGITS (&roots[i]) = NULL;
  }
  
  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (points_in = MPArray_init (nPoints));
  P_CHECKA (points_out = MPArray_init (nPoints));
  MP_CHECKC (mp_init (&should_be));
  MP_CHECKC (mp_init (&tmp));

  fft_get_roots (roots, nPoints, cfg, false);

  mp_set (&points_in->data[0], 3);
  mp_set (&points_in->data[1], 8);
  mp_set (&points_in->data[2], 7);
  mp_set (&points_in->data[3], 9);
  mu_check (fft (points_out, points_in, cfg, false) == SECSuccess);

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

cleanup:
  mu_check (rv == SECSuccess);
  mp_clear (&tmp);
  mp_clear (&should_be);
  MPArray_clear (points_in);
  MPArray_clear (points_out);
  PrioConfig_clear (cfg);
}

void 
mu_test__fft_invert (void)
{
  SECStatus rv = SECSuccess;
  const int nPoints = 8;

  PrioConfig cfg = NULL;
  MPArray points_in = NULL;
  MPArray points_out = NULL;
  MPArray points_out2 = NULL;
  mp_int roots[nPoints];
  
  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (points_in = MPArray_init (nPoints)); 
  P_CHECKA (points_out = MPArray_init (nPoints));
  P_CHECKA (points_out2 = MPArray_init (nPoints));

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

cleanup:
  mu_check (rv == SECSuccess);

  MPArray_clear (points_in);
  MPArray_clear (points_out);
  MPArray_clear (points_out2);
  PrioConfig_clear (cfg);
}
