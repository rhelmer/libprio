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

#include "rand.h"

#include "include/prio.h"
#include <nss/pk11pub.h>

int 
rand_int (mp_int *out, const mp_int *max)
{
  do {
    const int nbits = mp_raw_size (max);
    const bool mul_of_byte = nbits % sizeof (unsigned char);
    const int byte_len = nbits / sizeof (unsigned char);
    const int nbytes = mul_of_byte ? byte_len : byte_len + 1;

    unsigned char rand_bytes[nbytes+1];
    // First byte in mpi determines sign of integer.
    // Zero byte means positive.
    rand_bytes[0] = '\0';
    if (PK11_GenerateRandom(rand_bytes+1, nbytes) != SECSuccess)
      return PRIO_ERROR;

    if (mp_read_raw (out, (char *)rand_bytes, nbytes+1) != MP_OKAY)
      return PRIO_ERROR;

    // Use [inefficient] rejection sampling to find a number
    // strictly less than max.
    // TODO: Optimize to make fewer iterations. 
  } while (mp_cmp (out, max) != -1);

  return 0;
}


