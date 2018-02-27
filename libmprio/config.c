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

#include <stdlib.h>
#include <mprio.h>

#include "config.h"
#include "params.h"
#include "rand.h"
#include "util.h"


static int
initialize_roots (struct mparray *arr, const char *values[])
{
  // TODO: Read in only the number of roots of unity we need.
  // Right now we read in all 4096 roots whether or not we use
  // them all.
  for (int i=0; i < arr->len; i++) {
    if ((mp_read_radix (&arr->data[i], values[i], 16) != MP_OKAY)) 
      return PRIO_ERROR;
  }
  
  return PRIO_OKAY;
}

PrioConfig 
PrioConfig_defaultNew (void)
{
  PrioConfig cfg = malloc (sizeof (*cfg));
  if (!cfg)
    return NULL;

  MP_CHECKN (mp_init (&cfg->modulus));
  MP_CHECKN (mp_init (&cfg->inv2));

  MP_CHECKN (mp_read_radix (&cfg->modulus, Modulus, 16)); 

  // Compute  2^{-1} modulo M
  mp_set (&cfg->inv2, 2);
  MP_CHECKN (mp_invmod (&cfg->inv2, &cfg->modulus, &cfg->inv2)); 

  cfg->num_data_fields = DefaultNumDataFields;
  cfg->n_roots = 1 << Generator2Order;
  if (cfg->num_data_fields >= cfg->n_roots)
    return NULL;

  MP_CHECKN (mparray_init (&cfg->roots, cfg->n_roots)); 
  MP_CHECKN (initialize_roots (&cfg->roots, Roots)); 
  MP_CHECKN (mparray_init (&cfg->rootsInv, cfg->n_roots)); 
  MP_CHECKN (initialize_roots (&cfg->rootsInv, RootsInv)); 

  return cfg;
}

void 
PrioConfig_clear(PrioConfig cfg)
{
  mparray_clear (&cfg->roots);
  mparray_clear (&cfg->rootsInv);
  mp_clear (&cfg->modulus);
  mp_clear (&cfg->inv2);
  free (cfg);
}

int 
PrioConfig_numDataFields (const_PrioConfig cfg)
{
  return cfg->num_data_fields;
}
