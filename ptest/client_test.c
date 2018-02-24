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
  printf("Her");
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

  PrioPacketClient_clear (cfg, pA);
  PrioPacketClient_clear (cfg, pB);
  PrioConfig_clear (cfg);
}

