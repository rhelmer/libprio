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
#include "libmprio/util.h"
#include "mutest.h"

void 
mu_test_client__new (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  PrioPacketClient pA = NULL;
  PrioPacketClient pB = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (pA = PrioPacketClient_new (cfg));
  P_CHECKA (pB = PrioPacketClient_new (cfg));

  {
  const int ndata = PrioConfig_numDataFields (cfg);
  bool data_items[ndata];

  for (int i=0; i < ndata; i++) {
    // Arbitrary data
    data_items[i] = (i % 3 == 1) || (i % 5 == 3);
  }

  P_CHECKC (PrioPacketClient_set_data (cfg, data_items, pA, pB));
  }

cleanup:
  mu_check (rv == SECSuccess);

  PrioPacketClient_clear (pA);
  PrioPacketClient_clear (pB);
  PrioConfig_clear (cfg);
}

void 
test_client_agg (int nclients)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  PrioServer sA = NULL;
  PrioServer sB = NULL;
  PrioTotalShare tA = NULL;
  PrioTotalShare tB = NULL;
  PrioPacketClient pA = NULL;
  PrioPacketClient pB = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (sA = PrioServer_new (cfg, 0));
  P_CHECKA (sB = PrioServer_new (cfg, 1));
  P_CHECKA (pA = PrioPacketClient_new (cfg));
  P_CHECKA (pB = PrioPacketClient_new (cfg));

  const int ndata = PrioConfig_numDataFields (cfg);

  {
    bool data_items[ndata];
    for (int i=0; i < ndata; i++) {
      // Arbitrary data
      data_items[i] = (i % 3 == 1) || (i % 5 == 3);
    }

    mu_ensure (sA);
    mu_ensure (sB);

    for (int i=0; i < nclients; i++) {
      P_CHECKC (PrioPacketClient_set_data (cfg, data_items, pA, pB));
      mu_check (PrioServer_aggregate (sA, pA) == SECSuccess);
      mu_check (PrioServer_aggregate (sB, pB) == SECSuccess);
    }

    P_CHECKA (tA = PrioTotalShare_new (sA));
    P_CHECKA (tB = PrioTotalShare_new (sB));

    unsigned long output[ndata];
    mu_check (PrioTotalShare_final (cfg, output, tA, tB) == SECSuccess);
    for (int i=0; i < ndata; i++) {
      unsigned long v = ((i % 3 == 1) || (i % 5 == 3));
      mu_check (output[i] == v*nclients);
    }

  }

  //rv = SECFailure;
  //goto cleanup;

cleanup:
  mu_check (rv == SECSuccess);
  PrioPacketClient_clear (pA);
  PrioPacketClient_clear (pB);

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
mu_test_client__agg_10 (void)
{
  test_client_agg (10);
}

