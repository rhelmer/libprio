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

#include <mprio.h>
#include "libmpi/mpi.h"

struct mparray {
  int len;
  mp_int *data;
};

int mparray_init (struct mparray *arr, int len);

// Initializes two arrays and copies into them a secret sharing of 
// the array in src.
int mparray_init_share (struct mparray *arrA, struct mparray *arrB, 
    const struct mparray *src, const_PrioConfig cfg);

// Initializes array with 0/1 values specified in boolean array `data_in`
int mparray_init_bool (struct mparray *arr, int len, const bool *data_in);

// Expands or shrinks the mparray to the desired size. If shrinking,
// will clear the values on the end of array.
int mparray_resize (struct mparray *arr, int newlen);

// Initializes dst and creates a duplicate of the array in src. 
int mparray_dup (struct mparray *dst, const struct mparray *src);

// For each index i into the array, set:
//    dst[i] = dst[i] + to_add[i]   (modulo mod)
int mparray_addmod (struct mparray *dst, const struct mparray *to_add, 
    const mp_int *mod);
void mparray_clear (struct mparray *arr);

#endif /* __MPARRAY_H__ */

