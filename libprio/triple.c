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

#include "include/prio.h"

#include "config.h"
#include "rand.h"
#include "triple.h"

int 
triple_new (struct beaver_triple *triple)
{
  int error;

  if ((error = mp_init (&triple->a)) != MP_OKAY)
    return error;
  if ((error = mp_init (&triple->b)) != MP_OKAY)
    return error;
  if ((error = mp_init (&triple->c)) != MP_OKAY)
    return error;

  return PRIO_OKAY;
}


void
triple_clear (struct beaver_triple *triple)
{
  mp_clear (&triple->a);
  mp_clear (&triple->b);
  mp_clear (&triple->c);
}

int 
triple_rand (const struct prio_config *cfg, 
    struct beaver_triple *triple_1, 
    struct beaver_triple *triple_2)
{
  int error;

  // We need that
  //   (a1 + a2)(b1 + b2) = c1 + c2   (mod p) 
  if ((error = rand_int (&triple_1->a, &cfg->modulus)) != PRIO_OKAY)
    return error;
  if ((error = rand_int (&triple_1->b, &cfg->modulus)) != PRIO_OKAY)
    return error;
  if ((error = rand_int (&triple_2->a, &cfg->modulus)) != PRIO_OKAY)
    return error;
  if ((error = rand_int (&triple_2->b, &cfg->modulus)) != PRIO_OKAY)
    return error;

  // We are trying to be a little clever here to avoid the use of temp
  // variables.

  // c1 = a1 + a2
  if ((error = mp_addmod (&triple_1->a, &triple_2->a, &cfg->modulus, &triple_1->c)) != PRIO_OKAY)
    return error;

  // c2 = b1 + b2
  if ((error = mp_addmod (&triple_1->b, &triple_2->b, &cfg->modulus, &triple_2->c)) != PRIO_OKAY)
    return error;

  // c1 = c1 * c2 = (a1 + a2) (b1 + b2)
  if ((error = mp_mulmod (&triple_1->c, &triple_2->c, &cfg->modulus, &triple_1->c)) != PRIO_OKAY)
    return error;

  // Set c2 to random blinding value
  if ((error = rand_int (&triple_2->c, &cfg->modulus)) != PRIO_OKAY)
    return error;

  // c1 = c1 - c2
  if ((error = mp_submod (&triple_1->c, &triple_2->c, &cfg->modulus, &triple_1->c)) != PRIO_OKAY)
    return error;

  // Now we should have random tuples satisfying:
  //   (a1 + a2) (b1 + b2) = c1 + c2

  return PRIO_OKAY;
}
