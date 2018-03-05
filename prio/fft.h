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

#ifndef _FFT__H
#define _FFT__H

#include <mprio.h>
#include <stdbool.h>

#include "mpi/mpi.h"
#include "mparray.h"

/*
 * Compute the FFT or inverse FFT of the array in `points_in`.
 * The length of the input and output arrays must be a multiple
 * of two and must be no longer than the number of precomputed
 * roots in the PrioConfig object passed in.
 */
SECStatus fft(MPArray points_out, const_MPArray points_in, 
    const_PrioConfig cfg, bool invert);

/* 
 * Get an array
 *    (r^0, r^1, r^2, ... ) 
 * where r is an n-th root of unity, for n a power of two
 * less than cfg->n_roots.
 *
 * Do NOT mp_clear() the mp_ints stored in roots_out.  
 * These are owned by the PrioConfig object.
 */
SECStatus fft_get_roots (mp_int *roots_out, int n_points,
    const_PrioConfig cfg, bool invert);

#endif
