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
#include "mparray.h"
#include "server.h"

PrioServer 
PrioServer_new (const_PrioConfig cfg)
{
  PrioServer s = malloc (sizeof (*s));
  if (!s) return NULL;
  s->cfg = cfg;
  
  if (mparray_init (&s->data_shares, s->cfg->num_data_fields) != PRIO_OKAY)
    return NULL;

  return s;
}

void 
PrioServer_clear (PrioServer s)
{
  mparray_clear (&s->data_shares);
  free(s);
}

int 
PrioServer_aggregate (PrioServer s, const_PrioPacketClient p)
{
  return mparray_addmod (&s->data_shares, &p->data_shares, &s->cfg->modulus);  
}

PrioTotalShare 
PrioTotalShare_new (const_PrioServer s)
{
  PrioTotalShare t = malloc (sizeof (*t));
  if (!t) return NULL;
  
  if (mparray_dup (&t->data_shares, &s->data_shares) != PRIO_OKAY)
    return NULL;

  return t;
}

int PrioTotalShare_final (const_PrioConfig cfg, 
    unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB)
{
  mp_int tmp;
  if (mp_init (&tmp) != MP_OKAY)
    return PRIO_ERROR;
  if (tA->data_shares.len != cfg->num_data_fields)
    return PRIO_ERROR;
  if (tA->data_shares.len != tB->data_shares.len)
    return PRIO_ERROR;

  for (int i=0; i<cfg->num_data_fields; i++) {
    if (mp_addmod(&tA->data_shares.data[i], &tB->data_shares.data[i], 
          &cfg->modulus, &tmp) != MP_OKAY)
      return PRIO_ERROR;

    output[i] = tmp.dp[0];
  }

  mp_clear (&tmp);
  return PRIO_OKAY;
}

void 
PrioTotalShare_clear (PrioTotalShare t)
{
  mparray_clear (&t->data_shares);
  free (t);
}

PrioVerifier PrioVerifier_new (PrioServer s, const_PrioPacketClient p, 
    char shared_secret[16])
{
  PrioVerifier v = malloc (sizeof *v);
  if (!v) return NULL;
  v->cfg = s->cfg;
  v->c_packet = p;

  const int n = v->cfg->num_data_fields + 1;
  const int N = next_power_of_two (n);

  // Use shared secret to compute point r

  // Compute share of f(r), g(r), h(r)

  // Check that all values that should be zero are zero
}

void PrioVerifier_clear (PrioVerifier v)
{
  free (v);
}

/*
PrioPacketVerify1 PrioVerifier_packet1 (const_PrioVerifier v);
void PrioPacketVerify1_clear (PrioPacketVerify1 p);

PrioPacketVerify2 PrioVerifier_packet2 (const_PrioVerifier v,
    PrioPacketVerify1 pA, PrioPacketVerify1 pB);
int PrioVerifier_isValid (const_PrioVerifier v,
    PrioPacketVerify2 pA, PrioPacketVerify2 pB);
void PrioPacketVerify2_clear (PrioPacketVerify2 p);
*/

