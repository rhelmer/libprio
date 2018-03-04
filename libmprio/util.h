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


#ifndef __UTIL_H__
#define __UTIL_H__

#include <mprio.h>
#include "libmpi/mpi.h"

// Check a Prio error code and return failure if the call fails.
#define P_CHECK(s) \
  do { \
    if((rv = (s)) != SECSuccess) \
    return rv; \
  } while(0);

// Check an allocation that should not return NULL. If the allocation returns
// NULL, set the return value and jump to the cleanup label to free memory. 
#define P_CHECKA(s) \
  do { \
    if((s) == NULL) {\
      rv = SECFailure;\
      goto cleanup;\
    }\
  } while(0);

// Check a Prio library call that should return SECSuccess. If it doesn't,
// jump to the cleanup label.
#define P_CHECKC(s) \
  do { \
    if((rv = (s)) != SECSuccess) { \
       goto cleanup; \
    }\
  } while(0);

// Check an MPI library call and return failure if it fails.
#define MP_CHECK(s) do { if((s) != MP_OKAY) return SECFailure; } while(0);

// Check an MPI library call. If it fails, set the return code and jump
// to the cleanup label.
#define MP_CHECKC(s) \
  do { \
    if((s) != MP_OKAY) { \
       rv = SECFailure; \
       goto cleanup; \
    }\
  } while(0);

inline int
next_power_of_two (int val)
{
  int i = val;
  int out = 0;
  for ( ; i > 0; i >>= 1) {
    out++;
  }

  int pow = 1 << out;
  return (pow > 1 && pow/2 == val) ? val : pow;
} 

/* 
 * Return a mask that masks out all of the zero bits
 */
inline unsigned char
msb_mask (unsigned char val)
{
  unsigned char mask;
  for (mask = 0x00; (val & mask) != val; mask = (mask << 1) + 1);
  return mask;
}

#endif /* __UTIL_H__ */

