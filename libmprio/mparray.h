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


#ifndef __MPARRAY_H__
#define __MPARRAY_H__

struct mparray {
  int len;
  mp_int *data;
};

typedef struct mparray *MPArray;
typedef const struct mparray *const_MPArray;

#include <mprio.h>
#include "libmpi/mpi.h"

MPArray MPArray_init (int len);

// Initializes two arrays and copies into them a secret sharing of 
// the array in src.
SECStatus MPArray_init_share (MPArray *arrA, MPArray *arrB, 
    const_MPArray src, const_PrioConfig cfg);

// Initializes array with 0/1 values specified in boolean array `data_in`
MPArray MPArray_init_bool (int len, const bool *data_in);

// Expands or shrinks the MPArray to the desired size. If shrinking,
// will clear the values on the end of array.
SECStatus MPArray_resize (MPArray arr, int newlen);

// Initializes dst and creates a duplicate of the array in src. 
MPArray MPArray_dup (const_MPArray src);

// For each index i into the array, set:
//    dst[i] = dst[i] + to_add[i]   (modulo mod)
SECStatus MPArray_addmod (MPArray dst, const_MPArray to_add, 
    const mp_int *mod);
void MPArray_clear (MPArray arr);

#endif /* __MPARRAY_H__ */

