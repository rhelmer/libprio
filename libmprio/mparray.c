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

int
mparray_init (struct mparray *arr, int len)
{
  arr->len = len;
  arr->data = calloc (len, sizeof (mp_int));
  if (!arr->data) 
    return PRIO_ERROR;

  for (int i=0; i<len; i++) {
    if (mp_init(&arr->data[i]) != MP_OKAY)
      return PRIO_ERROR;
  }

  return PRIO_OKAY;
}

int 
mparray_dup (struct mparray *dst, const struct mparray *src)
{
  mparray_init (dst, src->len);
  for (int i=0; i<src->len; i++) {
    if (mp_copy(&src->data[i], &dst->data[i]) != MP_OKAY)
      return PRIO_ERROR;
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
    if (mp_addmod (&dst->data[i], &to_add->data[i], mod, &dst->data[i]) != MP_OKAY)
      return PRIO_ERROR;
  }

  return PRIO_OKAY;
}

