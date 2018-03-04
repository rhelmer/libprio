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

#include <limits.h>
#include <nss/nss.h>
#include <nss/pk11pub.h>
#include <nspr/prinit.h>
#include <mprio.h>

#include "debug.h"
#include "rand.h"
#include "util.h"


SECStatus
rand_init (void)
{
  SECStatus rv = NSS_NoDB_Init (NULL);
  if (rv != SECSuccess) 
    return SECFailure;

  return SECSuccess;
}

SECStatus 
rand_bytes (unsigned char *out, int n_bytes)
{
  if (!NSS_IsInitialized ()) {
    PRIO_DEBUG ("NSS not initialized. Call rand_init() first.");
    return SECFailure;
  }

  SECStatus rv;
  if ((rv = PK11_GenerateRandom (out, n_bytes)) != SECSuccess) 
  {
    PRIO_DEBUG ("Error calling PK11_GenerateRandom");
    return SECFailure;
  }

  return rv;
}

SECStatus
rand_int (mp_int *out, const mp_int *max)
{
  SECStatus rv = SECSuccess;

  // Ensure max value is > 0
  if (mp_cmp_z (max) == 0)
    return SECFailure;

  // Compute max-1, which tells us the largest
  // value we will ever need to generate.
  MP_CHECK (mp_sub_d (max, 1, out));

  const int nbytes = mp_unsigned_octet_size (out);

  // Figure out how many MSBs we need to get in the 
  // most-significant byte. 
  unsigned char max_bytes[nbytes];
  MP_CHECK (mp_to_fixlen_octets (out, max_bytes, nbytes));
  const unsigned char mask = msb_mask (max_bytes[0]); 

  // Buffer to store the pseudo-random bytes
  unsigned char buf[nbytes];

  do {
    // Use  rejection sampling to find a value strictly less than max.
    P_CHECK (rand_bytes (buf, nbytes));

    // Mask off high-order bits that we will never need.
    P_CHECK (rand_bytes (&buf[0], 1));
    if (mask) buf[0] &= mask;

    MP_CHECK (mp_read_unsigned_octets (out, buf, nbytes));
  } while (mp_cmp (out, max) != -1);

  return 0;
}

void
rand_clear (void)
{
  NSS_Shutdown ();
  PR_Cleanup ();
}

