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


#ifndef __PRG_H__
#define __PRG_H__

#include <nss/blapi.h>
#include <stdlib.h>

#include "mpi/mpi.h"
#include "config.h"

typedef struct prg *PRG;
typedef const struct prg *const_PRG;

/* 
 * Initialize or destroy a pseudo-random generator.
 */
PRG PRG_new (const PrioPRGSeed key);
void PRG_clear (PRG prg);

/* 
 * Produce the next bytes of output from the PRG.
 */
SECStatus PRG_get_bytes (PRG prg, unsigned char *bytes, size_t len);

/*
 * Use the PRG output to sample a big integer x in the range
 *    0 <= x < max.
 */
SECStatus PRG_get_int (PRG prg, mp_int *out, const mp_int *max);

/*
 * Use secret sharing to split the int src into two shares.
 * Use PRG to generate the value `shareB`.
 * The mp_ints must be initialized.
 */
SECStatus PRG_share_int (PRG prg, mp_int *shareA, const mp_int *src,
    const_PrioConfig cfg);

/* 
 * Set each item in the array to a pseudorandom value in the range
 * [0, mod), where the values are generated using the PRG.
 */
SECStatus PRG_get_array (PRG prg, MPArray arr, const mp_int *mod);

/* 
 * Secret shares the array in `src` into `arrA` using randomness
 * provided by `prgB`. The arrays `src` and `arrA` must be the same
 * length.
 */
SECStatus PRG_share_array (PRG prgB, MPArray arrA, 
    const_MPArray src, const_PrioConfig cfg);


#endif /* __PRG_H__ */

