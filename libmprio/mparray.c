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
MPArray_new (int len)
{
  SECStatus rv = SECSuccess;
  MPArray arr = malloc (sizeof *arr);
  if (!arr) 
    return NULL;

  arr->data = NULL;
  arr->len = len;

  P_CHECKA(arr->data = calloc (len, sizeof (mp_int)));

  // Initialize these to NULL so that we can figure
  // out which allocations failed (if any)
  for (int i=0; i<len; i++) {
    MP_DIGITS (&arr->data[i]) = NULL;
  }

  for (int i=0; i<len; i++) {
    MP_CHECKC (mp_init(&arr->data[i]));
  }

cleanup:
  if (rv != SECSuccess) {
    MPArray_clear (arr);
    return NULL;
  }

  return arr;
}

MPArray
MPArray_new_bool (int len, const bool *data_in)
{
  MPArray arr = MPArray_new (len);
  if (arr == NULL) return NULL;

  for (int i=0; i<len; i++) {
    mp_set (&arr->data[i], data_in[i]);
  }
 
  return arr;
}

SECStatus
MPArray_resize (MPArray arr, int newlen)
{
  SECStatus rv = SECSuccess;
  const int oldlen = arr->len;

  if (oldlen == newlen)
    return rv;  

  // TODO: Use realloc for this?
  mp_int *newdata = calloc (newlen, sizeof (mp_int));
  if (newdata == NULL)
    return SECFailure;

  for (int i = 0; i < newlen; i++) {
    MP_DIGITS (&newdata[i]) = NULL;
  }

  // Initialize new array
  for (int i = 0; i < newlen; i++) {
    MP_CHECKC (mp_init (&newdata[i]));
  }

  // Copy old data into new array
  for (int i = 0; i < newlen && i < oldlen; i++) {
    MP_CHECKC (mp_copy (&arr->data[i], &newdata[i]));
  }

  // Free old data
  for (int i = 0; i < oldlen; i++) {
    mp_clear (&arr->data[i]);
  }
  free (arr->data);
  arr->data = newdata;
  arr->len = newlen;

cleanup:
  if (rv != SECSuccess) {
    for (int i=0; i < newlen; i++) {
      mp_clear (&newdata[i]);
    }
    free (newdata);
  }

  return rv;
}

MPArray
MPArray_dup (const_MPArray src)
{
  MPArray dst = MPArray_new (src->len); 

  for (int i=0; i<src->len; i++) {
    if (mp_copy(&src->data[i], &dst->data[i]) != MP_OKAY) {
      MPArray_clear (dst);
      return NULL;
    }
  }

  return dst;
}

SECStatus
MPArray_set_share (MPArray arrA, MPArray arrB, 
    const_MPArray src, const_PrioConfig cfg)
{
  SECStatus rv;
  if (arrA->len != src->len || arrB->len != src->len)
    return SECFailure;

  const int len = src->len;

  for (int i=0; i < len; i++) {
    P_CHECK(share_int(cfg, &src->data[i], &arrA->data[i], &arrB->data[i])); 
  }

  return rv;
}

void 
MPArray_clear (MPArray arr)
{
  if (arr == NULL) return;

  if (arr->data != NULL) {
    for (int i=0; i<arr->len; i++) {
      mp_clear(&arr->data[i]);
    }
    free (arr->data); 
  }
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

