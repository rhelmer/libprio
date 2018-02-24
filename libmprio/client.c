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

#include <stdio.h>
#include <stdlib.h>
#include <mprio.h>

#include "libmpi/mpi.h"
#include "client.h"

int 
PrioPacketClient_new (const_PrioConfig cfg, const bool *data_in,
    PrioPacketClient *ptrA, PrioPacketClient *ptrB)
{
  int error;

  if (!data_in || !ptrA || !ptrB)
    return PRIO_ERROR; 

  *ptrA = malloc (sizeof (**ptrA));
  if (!*ptrA) return PRIO_ERROR;
  *ptrB = malloc (sizeof (**ptrB));
  if (!*ptrB) return PRIO_ERROR;

  PrioPacketClient pA = *ptrA;
  PrioPacketClient pB = *ptrB;

  if ((error = triple_new (&pA->triple)) != PRIO_OKAY)
    return error;
  if ((error = triple_new (&pB->triple)) != PRIO_OKAY)
    return error;

  if ((error = triple_rand (cfg, &pA->triple, &pB->triple)) != PRIO_OKAY)
    return error;

  if ((error = mparray_init (&pA->data_shares, cfg->num_data_fields)) != PRIO_OKAY)
    return PRIO_ERROR;
  if ((error = mparray_init (&pB->data_shares, cfg->num_data_fields)) != PRIO_OKAY)
    return PRIO_ERROR;

  mp_int tmp;
  if ((error = mp_init (&tmp)) != MP_OKAY)
    return PRIO_ERROR;

  for (int i=0; i<cfg->num_data_fields; i++) {
    if ((error = mp_set_int (&tmp, data_in[i])) != MP_OKAY)
      return PRIO_ERROR;
    if (share_int(cfg, &tmp, &pA->data_shares.data[i], &pB->data_shares.data[i]) != MP_OKAY)
      return PRIO_ERROR;
  }

  mp_clear (&tmp);

  return PRIO_OKAY;
}

void
PrioPacketClient_clear (PrioPacketClient p)
{
  mparray_clear (&p->data_shares);
  triple_clear (&p->triple);
  free (p);
}

