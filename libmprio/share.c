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

#include "config.h"
#include "rand.h"
#include "share.h"
#include "util.h"


SECStatus
share_int (const struct prio_config *cfg, const mp_int *src, 
    mp_int *shareA, mp_int *shareB)
{
  SECStatus rv;
  P_CHECK (rand_int (shareA, &cfg->modulus)); 
  MP_CHECK (mp_submod (src, shareA, &cfg->modulus, shareB));

  return SECSuccess;
}

SECStatus
triple_new (struct beaver_triple *triple)
{
  MP_DIGITS (&triple->a) = NULL;
  MP_DIGITS (&triple->b) = NULL;
  MP_DIGITS (&triple->c) = NULL;

  SECStatus rv;
  MP_CHECKC (mp_init (&triple->a)); 
  MP_CHECKC (mp_init (&triple->b)); 
  MP_CHECKC (mp_init (&triple->c)); 

  return SECSuccess;

cleanup:
  mp_clear (&triple->a);
  mp_clear (&triple->b);
  mp_clear (&triple->c);
  return rv;
}


void
triple_clear (struct beaver_triple *triple)
{
  mp_clear (&triple->a);
  mp_clear (&triple->b);
  mp_clear (&triple->c);
}

SECStatus
triple_rand (const struct prio_config *cfg, 
    struct beaver_triple *triple_1, 
    struct beaver_triple *triple_2)
{
  SECStatus rv;

  // TODO: Can shorten this code using share_int()

  // We need that
  //   (a1 + a2)(b1 + b2) = c1 + c2   (mod p) 
  P_CHECK (rand_int (&triple_1->a, &cfg->modulus)); 
  P_CHECK (rand_int (&triple_1->b, &cfg->modulus)); 
  P_CHECK (rand_int (&triple_2->a, &cfg->modulus)); 
  P_CHECK (rand_int (&triple_2->b, &cfg->modulus)); 

  // We are trying to be a little clever here to avoid the use of temp
  // variables.

  // c1 = a1 + a2
  MP_CHECK (mp_addmod (&triple_1->a, &triple_2->a, &cfg->modulus, &triple_1->c));

  // c2 = b1 + b2
  MP_CHECK (mp_addmod (&triple_1->b, &triple_2->b, &cfg->modulus, &triple_2->c)); 

  // c1 = c1 * c2 = (a1 + a2) (b1 + b2)
  MP_CHECK (mp_mulmod (&triple_1->c, &triple_2->c, &cfg->modulus, &triple_1->c)); 

  // Set c2 to random blinding value
  MP_CHECK (rand_int (&triple_2->c, &cfg->modulus)); 

  // c1 = c1 - c2
  MP_CHECK (mp_submod (&triple_1->c, &triple_2->c, &cfg->modulus, &triple_1->c)); 

  // Now we should have random tuples satisfying:
  //   (a1 + a2) (b1 + b2) = c1 + c2

  return SECSuccess;
}
