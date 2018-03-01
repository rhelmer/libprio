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
#include <stdio.h>
#include <stdlib.h>

#include "libmprio/rand.h"
#include "libmprio/util.h"

void
verify_full (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  PrioServer sA = NULL;
  PrioServer sB = NULL;
  PrioPacketClient pA = NULL;
  PrioPacketClient pB = NULL;
  PrioVerifier vA = NULL;
  PrioVerifier vB = NULL;
  PrioPacketVerify1 p1A = NULL;
  PrioPacketVerify1 p1B = NULL;
  PrioPacketVerify2 p2A = NULL;
  PrioPacketVerify2 p2B = NULL;
  PrioTotalShare tA = NULL;
  PrioTotalShare tB = NULL;

  // Number of different boolean data fields we collect
  const int ndata = 100;

  // New scope to avoid goto weirdness
  {
    bool data_items[ndata];

    // Use the default configuration parameters.
    P_CHECKA (cfg = PrioConfig_new (ndata));

    // The client's data submission is an arbitrary
    // boolean vector.
    for (int i=0; i < ndata; i++) {
      // Arbitrary data
      data_items[i] = (i % 3 == 1) || (i % 5 == 3);
    }

    // Initialize two server objects.
    P_CHECKA (sA = PrioServer_new (cfg, 0));
    P_CHECKA (sB = PrioServer_new (cfg, 1));

    // Initize two empty client data packets.
    P_CHECKA (pA = PrioPacketClient_new (cfg));
    P_CHECKA (pB = PrioPacketClient_new (cfg));

    // Construct the client data packets.
    P_CHECKC (PrioPacketClient_set_data (cfg, data_items, pA, pB));

    // THE CLIENT'S JOB IS DONE. The rest of the 
    // processing just takes place between the
    // two servers.


    // The servers must generate a new shared secret to check
    // each client request. (Reusing the same randomness to
    // check multiple requests is NOT safe.) The servers can
    // use a PRG (e.g., AES in counter mode) to generate many
    // shared secrets from a short (e.g., 128-bit) seed.
    ServerSharedSecret sec;
    P_CHECKC (rand_bytes (sec, SOUNDNESS_PARAM));

    // Use the shared secret between the servers to set up 
    // a Prio verifier object.
    P_CHECKA (vA = PrioVerifier_new (sA, pA, sec));
    P_CHECKA (vB = PrioVerifier_new (sB, pB, sec));

    // Both servers produce a packet1
    P_CHECKA (p1A = PrioVerifier_packet1(vA));
    P_CHECKA (p1B = PrioVerifier_packet1(vB));

    // Both servers produce a packet2
    P_CHECKA (p2A = PrioVerifier_packet2(vA, p1A, p1B));
    P_CHECKA (p2B = PrioVerifier_packet2(vB, p1A, p1B));

    // The output of packet2 lets the servers determine
    // whether the request is valid.
    P_CHECKC (PrioVerifier_isValid (vA, p2A, p2B)); 
    P_CHECKC (PrioVerifier_isValid (vB, p2A, p2B)); 

    // If we get here, the client packet is valid, so 
    // add it to the aggregate statistic counter for both
    // servers.
    P_CHECKC (PrioServer_aggregate (sA, pA));
    P_CHECKC (PrioServer_aggregate (sB, pB));

    // The servers repeat the steps above for each client
    // submission.

    // After collecting aggregates from MANY clients,
    // the servers can compute their shares of the aggregate
    // statistics. 
    P_CHECKA (tA = PrioTotalShare_new (sA));
    P_CHECKA (tB = PrioTotalShare_new (sB));

    // The servers put their two shares together to 
    // get the actual data.
    unsigned long output[ndata];
    P_CHECKC (PrioTotalShare_final (cfg, output, tA, tB));
    
    // Now the output[i] contains a counter that indicates
    // how many clients submitted TRUE for data value i. 
  }

cleanup:
  PrioTotalShare_clear (tA);
  PrioTotalShare_clear (tB);

  PrioPacketVerify2_clear (p2A);
  PrioPacketVerify2_clear (p2B);

  PrioPacketVerify1_clear (p1A);
  PrioPacketVerify1_clear (p1B);

  PrioVerifier_clear (vA);
  PrioVerifier_clear (vB);

  PrioPacketClient_clear (pA);
  PrioPacketClient_clear (pB);

  PrioServer_clear (sA);
  PrioServer_clear (sB);
  PrioConfig_clear (cfg);
}

int 
main (void)
{
  puts ("This utility demonstrates how to invoke the Prio API.");
  verify_full ();
  return 0;
}

