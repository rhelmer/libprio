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
#include "rand.h"


PrioConfig 
PrioConfig_defaultNew (void)
{
  PrioConfig cfg = malloc (sizeof (*cfg));
  if (!cfg)
    return NULL;

  if (mp_init (&cfg->modulus) != MP_OKAY)
    return NULL;

  if ((mp_read_radix(&cfg->modulus, "8000000000000000080001", 16) != MP_OKAY))
    return NULL;

  cfg->num_data_fields = 128;
  return cfg;
}

void 
PrioConfig_clear(PrioConfig cfg)
{
  mp_clear (&cfg->modulus);
  free (cfg);
}

int 
PrioConfig_numDataFields (const_PrioConfig cfg)
{
  return cfg->num_data_fields;
}
