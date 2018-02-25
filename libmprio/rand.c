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


int
rand_init (void)
{
  int error = NSS_NoDB_Init (".");
  if (error != SECSuccess) 
    return PRIO_ERROR;

  // For this example, we don't use database passwords
  PK11_InitPin(PK11_GetInternalKeySlot(), "", "");

  return PRIO_OKAY;
}

int 
rand_int (mp_int *out, const mp_int *max)
{
  if (!NSS_IsInitialized ()) {
    PRIO_DEBUG ("NSS not initialized. Call rand_init() first.");
    return PRIO_ERROR;
  }

  // Ensure max value is > 0
  if (mp_cmp_z (max) == 0)
    return PRIO_ERROR;

  // Compute max-1, which tells us the largest
  // value we will ever need to generate.
  if (mp_sub_d (max, 1, out) != MP_OKAY)
    return PRIO_ERROR;

  const int nbytes = mp_unsigned_octet_size (out);

  do {

    unsigned char rand_bytes[nbytes+1];
    // First byte in mpi determines sign of integer.
    // Zero byte means positive.
    rand_bytes[0] = '\0';
    int error;
    if ((error=PK11_GenerateRandom(rand_bytes+1, nbytes)) != SECSuccess) 
    {
      PRIO_DEBUG ("Error calling PK11_GenerateRandom");
      return PRIO_ERROR;
    }

    if (mp_read_raw (out, (char *)rand_bytes, nbytes+1) != MP_OKAY)
    {
      PRIO_DEBUG ("Error converting bytes to big integer");
      return PRIO_ERROR;
    }

    // Use [inefficient] rejection sampling to find a number
    // strictly less than max.
    // TODO: Optimize to make fewer iterations. 
    //mp_print (out, stderr);
    //puts("");
    //mp_print (max, stderr);
    //puts("");
    //puts("");
  } while (mp_cmp (out, max) != -1);

  //puts("DONE!");

  return 0;
}

void
rand_clear (void)
{
  NSS_Shutdown ();
  PR_Cleanup ();
}

