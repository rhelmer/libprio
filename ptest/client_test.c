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
#include "mutest.h"

void 
mu_test_client__new (void)
{
  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  const int ndata = PrioConfig_numDataFields (cfg);
  bool data_items[ndata];
  for (int i=0; i < ndata; i++) {
    // Arbitrary data
    data_items[i] = (i % 3 == 1) || (i % 5 == 3);
  }

  PrioPacketClient pA, pB;
  mu_check (PrioPacketClient_new (cfg, data_items, &pA, &pB) == PRIO_OKAY);

  PrioPacketClient_clear (pA);
  PrioPacketClient_clear (pB);
  PrioConfig_clear (cfg);
}

void 
test_client_agg (int nclients)
{
  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  const int ndata = PrioConfig_numDataFields (cfg);
  bool data_items[ndata];
  for (int i=0; i < ndata; i++) {
    // Arbitrary data
    data_items[i] = (i % 3 == 1) || (i % 5 == 3);
  }

  PrioServer sA = PrioServer_new (cfg);
  PrioServer sB = PrioServer_new (cfg);
  mu_check (sA);
  mu_check (sB);

  for (int i=0; i < nclients; i++) {
    PrioPacketClient pA, pB;
    mu_check (PrioPacketClient_new (cfg, data_items, &pA, &pB) == PRIO_OKAY);

    mu_check (PrioServer_aggregate (sA, pA) == PRIO_OKAY);
    mu_check (PrioServer_aggregate (sB, pB) == PRIO_OKAY);

    PrioPacketClient_clear (pA);
    PrioPacketClient_clear (pB);
  }

  PrioTotalShare tA = PrioTotalShare_new (sA);
  PrioTotalShare tB = PrioTotalShare_new (sB);
  mu_check (tA);
  mu_check (tB);

  unsigned long output[ndata];
  mu_check (PrioTotalShare_final (cfg, output, tA, tB) == PRIO_OKAY);
  for (int i=0; i < ndata; i++) {
    unsigned long v = ((i % 3 == 1) || (i % 5 == 3));
    mu_check (output[i] == v*nclients);
  }

  PrioTotalShare_clear (tA);
  PrioTotalShare_clear (tB);

  PrioServer_clear (sA);
  PrioServer_clear (sB);
  PrioConfig_clear (cfg);
}

void 
mu_test_client__agg_1 (void)
{
  test_client_agg (1);
}

void 
mu_test_client__agg_2 (void)
{
  test_client_agg (2);
}

void 
mu_test_client__agg_1000 (void)
{
  test_client_agg (1000);
}
