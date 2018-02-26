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
mu_test__verify_new (void)
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

  PrioPacketClient pA, pB;
  mu_check (PrioPacketClient_new (cfg, data_items, &pA, &pB) == PRIO_OKAY);

  ServerSharedSecret sec = { 
    0x12, 0x87, 0xd1, 0x12, 0x12, 
    0x12, 0x87, 0xd1, 0x32, 0x22, 
    0x12, 0x87, 0xd1, 0x18, 0x84, 
    0x12, 0x87, 0xd1, 0x12, 0x12 };
  PrioVerifier vA = PrioVerifier_new (sA, pA, sec);
  PrioVerifier vB = PrioVerifier_new (sB, pB, sec);
  mu_check (vA);
  mu_check (vB);



  PrioVerifier_clear (vA);
  PrioVerifier_clear (vB);

  PrioPacketClient_clear (pA);
  PrioPacketClient_clear (pB);

  PrioServer_clear (sA);
  PrioServer_clear (sB);
  PrioConfig_clear (cfg);
}

