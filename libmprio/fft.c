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

#include "config.h"
#include "fft.h"
#include "util.h"

static SECStatus
fft_recurse (mp_int *out, const mp_int *mod, int n, 
    const mp_int *roots, const mp_int *ys,
    mp_int *tmp, mp_int *ySub, mp_int *rootsSub)
{
  if (n == 1) {
    MP_CHECK (mp_copy (&ys[0], &out[0]));
    return SECSuccess;
  }

  // Recurse on the first half 
  for (int i=0; i<n/2; i++) {
    MP_CHECK (mp_addmod (&ys[i], &ys[i+(n/2)], mod, &ySub[i]));
    MP_CHECK (mp_copy (&roots[2*i], &rootsSub[i]));
  }

  MP_CHECK (fft_recurse (tmp, mod, n/2, rootsSub, ySub, &tmp[n/2], &ySub[n/2], &rootsSub[n/2]));
  for (int i=0; i<n/2; i++) {
    MP_CHECK (mp_copy (&tmp[i], &out[2*i]));
  }

  // Recurse on the second half 
  for (int i=0; i<n/2; i++) {
    MP_CHECK (mp_submod (&ys[i], &ys[i+(n/2)], mod, &ySub[i]));
    MP_CHECK (mp_mulmod (&ySub[i], &roots[i], mod, &ySub[i]));
  }

  MP_CHECK (fft_recurse (tmp, mod, n/2, rootsSub, ySub, &tmp[n/2], &ySub[n/2], &rootsSub[n/2]));
  for (int i=0; i<n/2; i++) {
    MP_CHECK (mp_copy (&tmp[i], &out[2*i + 1]));
  }

  return SECSuccess;
}

static SECStatus
fft_interpolate_raw (mp_int *out, 
    const mp_int *ys, int nPoints, const mp_int *roots, 
    const mp_int *mod, bool invert)
{
  SECStatus rv = SECSuccess;
  mp_int tmp[nPoints];
  mp_int ySub[nPoints];
  mp_int rootsSub[nPoints];
  for (int i=0; i<nPoints;i++) {
    MP_DIGITS (&tmp[i]) = NULL;
    MP_DIGITS (&ySub[i]) = NULL;
    MP_DIGITS (&rootsSub[i]) = NULL;
  }

  mp_int n_inverse;
  MP_DIGITS (&n_inverse) = NULL;

  for (int i=0; i<nPoints;i++) {
    MP_CHECKC (mp_init (&tmp[i]));
    MP_CHECKC (mp_init (&ySub[i]));
    MP_CHECKC (mp_init (&rootsSub[i]));
  }

  MP_CHECK (fft_recurse(out, mod, nPoints, roots, ys, tmp, ySub, rootsSub));

  if (invert) {
    MP_CHECKC (mp_init (&n_inverse));

    mp_set (&n_inverse, nPoints);
    MP_CHECKC (mp_invmod (&n_inverse, mod, &n_inverse));
    for (int i=0; i<nPoints;i++) {
      MP_CHECKC (mp_mulmod(&out[i], &n_inverse, mod, &out[i]));
      if (i == 3) {
        rv = SECFailure;
        goto cleanup;
      }
    }
  }

cleanup:
  mp_clear (&n_inverse); 
  for (int i=0; i<nPoints;i++) {
    mp_clear(&tmp[i]);
    mp_clear(&ySub[i]);
    mp_clear(&rootsSub[i]);
  }

  return rv;
} 

void
fft_get_roots (mp_int *roots_out, int n_points, const_PrioConfig cfg, bool invert)
{
  const mp_int *roots_in = invert ? cfg->rootsInv->data : cfg->roots->data;
  const int step_size = cfg->n_roots / n_points;

  for (int i=0; i < n_points; i++) {
    roots_out[i] = roots_in[i * step_size];
  }
}

SECStatus
fft (MPArray points_out, const_MPArray points_in, 
    const_PrioConfig cfg, bool invert)
{
  if (points_out->len != points_in->len)
    return SECFailure;

  const int n_points = points_in->len;
  if (cfg->n_roots % n_points != 0) 
    return SECFailure;

  mp_int scaled_roots[n_points];
  fft_get_roots (scaled_roots, n_points, cfg, invert);

  MP_CHECK (fft_interpolate_raw (points_out->data, points_in->data, n_points, 
      scaled_roots, &cfg->modulus, invert));

  return SECSuccess;
}


