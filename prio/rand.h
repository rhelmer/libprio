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


#ifndef __RAND_H__
#define __RAND_H__

#include <nss/seccomon.h>
#include <stdlib.h>
#include "mpi/mpi.h"

/*
 * Typedef for function pointer. A function pointer of type RandBytesFunc
 * points to a function that fills the buffer `out` of with `len` random bytes.
 */
typedef SECStatus (*RandBytesFunc) (void *user_data, unsigned char *out, size_t len);

/* 
 * Initialize or cleanup the global random number generator
 * state that NSS uses.
 */
SECStatus rand_init (void);
void rand_clear (void);

/* 
 * Generate the specified number of random bytes using the
 * NSS random number generator.
 */ 
SECStatus rand_bytes (unsigned char *out, size_t n_bytes);

/*
 * Generate a random number x such that
 *    0 <= x < max
 * using the NSS random number generator.
 */
SECStatus rand_int (mp_int *out, const mp_int *max);

/*
 * Generate a random number x such that
 *    0 <= x < max
 * using the specified randomness generator.
 *
 * The pointer user_data is passed to RandBytesFung `rng` as a first
 * argument.
 */
SECStatus rand_int_rng (mp_int *out, const mp_int *max, 
    RandBytesFunc rng, void *user_data);

#endif /* __RAND_H__ */

