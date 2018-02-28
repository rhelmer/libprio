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
#include <stdlib.h>

#include "config.h"
#include "mparray.h"
#include "share.h"
#include "util.h"

MPArray
MPArray_init (int len)
{
  bool okay = true;
  MPArray arr = malloc (sizeof *arr);
  if (!arr) 
    return NULL;

  arr->len = len;
  arr->data = calloc (len, sizeof (mp_int));
  if (!arr->data) {
    free (arr);
    return NULL;
  }

  // Initialize these to NULL so that we can figure
  // out which allocations failed (if any)
  for (int i=0; i<len; i++) {
    MP_DIGITS (&arr->data[i]) = NULL;
  }

  for (int i=0; i<len; i++) {
    if (mp_init(&arr->data[i]) != MP_OKAY) {
      okay = false;
      break;
    }
  }

  if (!okay) {
    for (int i=0; i<len; i++) {
      MPArray_clear (arr);
    }
    free (arr->data);
    free (arr);
  }

  return arr;
}

MPArray
MPArray_init_bool (int len, const bool *data_in)
{
  MPArray arr = MPArray_init (len);
  if (arr == NULL) return NULL;

  for (int i=0; i<len; i++) {
    mp_set (&arr->data[i], data_in[i]);
  }
 
  return arr;
}

SECStatus
MPArray_resize (MPArray arr, int newlen)
{
  const int oldlen = arr->len;

  // If shrinking array, free stuff at end of array.
  if (newlen < oldlen) {
    for (int i=newlen; i<oldlen; i++) {
      mp_clear (&arr->data[i]);
      MP_DIGITS (&arr->data[i]) = NULL;
    }
  }

  // Try to resize memory
  arr->len = newlen;
  void *ret = realloc (arr->data, sizeof (mp_int) * newlen);
  if (ret) {
    arr->data = ret;
  } else {
    // realloc failed... clear rest of array memory
    MPArray_clear (arr);
    return SECFailure;
  } 

  for (int i=oldlen; i<newlen; i++) {
    MP_DIGITS (&arr->data[i]) = NULL;
  }

  // Try to allocate new mp_ints at end of array
  for (int i=oldlen; i<newlen; i++) {
    if (mp_init(&arr->data[i]) != MP_OKAY) {
      MPArray_clear (arr);
      return SECFailure;
    }
  }

  return SECSuccess;
}

MPArray
MPArray_dup (const_MPArray src)
{
  MPArray dst = MPArray_init (src->len); 

  for (int i=0; i<src->len; i++) {
    if (mp_copy(&src->data[i], &dst->data[i]) != MP_OKAY) {
      MPArray_clear (dst);
      return NULL;
    }
  }

  return dst;
}

SECStatus
MPArray_init_share (MPArray *arrA, MPArray *arrB, 
    const_MPArray src, const_PrioConfig cfg)
{
  const int len = src->len;

  *arrA = MPArray_init (len);
  if (!*arrA) return SECFailure;

  *arrB = MPArray_init (len);
  if (!*arrB) { 
    MPArray_clear (*arrA);
    return SECFailure;
  }

  for (int i=0; i < len; i++) {
    if (share_int(cfg, &src->data[i], 
          &(*arrA)->data[i], &(*arrB)->data[i]) != SECSuccess) {
      MPArray_clear (*arrA);
      MPArray_clear (*arrB);
      return SECFailure;
    }
  }

  return SECSuccess;
}

void 
MPArray_clear (MPArray arr)
{
  for (int i=0; i<arr->len; i++) {
    mp_clear(&arr->data[i]);
  }
  free (arr->data);
  free (arr);
}

SECStatus
MPArray_addmod (MPArray dst, const_MPArray to_add, const mp_int *mod)
{
  if (dst->len != to_add->len)
    return SECFailure;

  for (int i=0; i<dst->len; i++) {
    MP_CHECK (mp_addmod (&dst->data[i], &to_add->data[i], mod, &dst->data[i])); 
  }

  return SECSuccess;
}

