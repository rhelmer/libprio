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
#include <strings.h>

#include "aes/aes.h"
#include "prg.h"
#include "rand.h"
#include "share.h"
#include "util.h"

// TODO: Replace this with the NSS AES implementation.
// As far as I know, the NSS AES APIs are not exported
// publicly, so I am using my own AES implementation for
// now.
struct prg {
  struct AES_ctx ctx;
};


SECStatus 
PRGSeed_randomize (PRGSeed *key)
{
  return rand_bytes ((unsigned char *)key, PRG_SEED_LENGTH);   
}

PRG 
PRG_new (const PRGSeed key)
{
  PRG prg = malloc (sizeof (*prg));
  if (!prg) return NULL;

  unsigned char iv[AES_BLOCK_SIZE];
  bzero (iv, AES_BLOCK_SIZE);
  AES_init_ctx_iv (&prg->ctx, key, iv);
    
  return prg;
}


void 
PRG_clear (PRG prg)
{
  if (!prg) return;
  free (prg);
}

static SECStatus 
PRG_get_bytes_internal (void *prg_vp, unsigned char *bytes, size_t len)
{ 
  PRG prg = (PRG)prg_vp;
  bzero (bytes, len);
  AES_CTR_xcrypt_buffer (&prg->ctx, bytes, len);
  return SECSuccess; 
}  

SECStatus 
PRG_get_bytes (PRG prg, unsigned char *bytes, size_t len)
{
  return PRG_get_bytes_internal ((void *)prg, bytes, len); 
}

SECStatus 
PRG_get_int (PRG prg, mp_int *out, const mp_int *max)
{
  return rand_int_rng (out, max, &PRG_get_bytes_internal, (void *)prg);
}

SECStatus
PRG_get_array (PRG prg, MPArray dst, const mp_int *mod)
{
  SECStatus rv;
  for (int i=0; i<dst->len; i++) {
    P_CHECK (PRG_get_int (prg, &dst->data[i], mod));
  }

  return SECSuccess;
}

SECStatus
PRG_share_int (PRG prgB, mp_int *shareA, const mp_int *src, const_PrioConfig cfg)
{
  SECStatus rv = SECSuccess;
  mp_int tmp;
  MP_DIGITS (&tmp) = NULL;

  MP_CHECKC (mp_init (&tmp));
  P_CHECKC (PRG_get_int (prgB, &tmp, &cfg->modulus)); 
  MP_CHECKC (mp_submod (src, &tmp, &cfg->modulus, shareA));

cleanup:
  mp_clear (&tmp);
  return rv;
}


SECStatus 
PRG_share_array (PRG prgB, MPArray arrA, 
    const_MPArray src, const_PrioConfig cfg)
{
  SECStatus rv = SECSuccess;
  if (arrA->len != src->len)
    return SECFailure;

  const int len = src->len;

  for (int i=0; i < len; i++) {
    P_CHECK(PRG_share_int (prgB, &arrA->data[i], &src->data[i], cfg));
  }

  return rv;

}
