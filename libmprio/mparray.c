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

int
mparray_init (struct mparray *arr, int len)
{
  arr->len = len;
  arr->data = calloc (len, sizeof (mp_int));
  if (!arr->data) 
    return PRIO_ERROR;

  for (int i=0; i<len; i++) {
    MP_CHECK (mp_init(&arr->data[i]));
  }

  return PRIO_OKAY;
}

int 
mparray_init_bool (struct mparray *arr, int len, const bool *data_in)
{
  int error;
  P_CHECK (mparray_init (arr, len)); 

  for (int i=0; i<len; i++) {
    mp_set (&arr->data[i], data_in[i]);
  }
 
  return PRIO_OKAY; 
}

int
mparray_resize (struct mparray *arr, int newlen)
{
  const int oldlen = arr->len;
  arr->len = newlen;

  if (newlen < oldlen) {
    for (int i=newlen; i<oldlen; i++) {
      mp_clear(&arr->data[i]);
    }
  }

  arr->data = realloc (arr->data, sizeof (mp_int) * newlen);
  if (!arr->data) 
    return PRIO_ERROR;

  for (int i=oldlen; i<newlen; i++) {
    MP_CHECK (mp_init(&arr->data[i]));
  }

  return PRIO_OKAY;
}

int 
mparray_dup (struct mparray *dst, const struct mparray *src)
{
  int error;
  P_CHECK (mparray_init (dst, src->len)); 

  for (int i=0; i<src->len; i++) {
    MP_CHECK (mp_copy(&src->data[i], &dst->data[i]));
  }

  return PRIO_OKAY;
}

int 
mparray_init_share (struct mparray *arrA, struct mparray *arrB, 
    const struct mparray *src, const_PrioConfig cfg)
{
  int error;
  const int len = src->len;
  P_CHECK (mparray_init (arrA, len));
  P_CHECK (mparray_init (arrB, len));

  for (int i=0; i < len; i++) {
    P_CHECK (share_int(cfg, &src->data[i], 
          &arrA->data[i], &arrB->data[i]));
  }

  return PRIO_OKAY;
}

void 
mparray_clear (struct mparray *arr)
{
  for (int i=0; i<arr->len; i++) {
    mp_clear(&arr->data[i]);
  }
  free (arr->data);
}

int 
mparray_addmod (struct mparray *dst, const struct mparray *to_add, const mp_int *mod)
{
  if (dst->len != to_add->len)
    return PRIO_ERROR;

  for (int i=0; i<dst->len; i++) {
    MP_CHECK (mp_addmod (&dst->data[i], &to_add->data[i], mod, &dst->data[i])); 
  }

  return PRIO_OKAY;
}

