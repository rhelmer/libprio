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

#include "libmpi/mpi.h"
#include "libmprio/server.h"
#include "libmprio/server.c"

void mu_test__eval_poly (void) 
{
  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  struct mparray coeffs;
  mu_ensure (mparray_init (&coeffs, 3) == PRIO_OKAY);
  mp_set (&coeffs.data[0], 2);
  mp_set (&coeffs.data[1], 8);
  mp_set (&coeffs.data[2], 3);

  mp_int eval_at, out;
  mu_ensure (mp_init (&eval_at) == MP_OKAY);
  mu_ensure (mp_init (&out) == MP_OKAY);
  mp_set (&eval_at, 7);

  const int val = 3*7*7 + 8*7 + 2;
  mu_ensure (eval_poly (&out, &coeffs, &eval_at, cfg) == PRIO_OKAY);

  mu_check (mp_cmp_d (&out, val) == 0);
  mp_clear (&out);
  mp_clear (&eval_at);
  mparray_clear (&coeffs);

  PrioConfig_clear (cfg);
}

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

  PrioServer sA = PrioServer_new (cfg, 0);
  PrioServer sB = PrioServer_new (cfg, 1);
  mu_check (sA);
  mu_check (sB);

  PrioPacketClient pA, pB;
  mu_check (PrioPacketClient_new (cfg, data_items, &pA, &pB) == PRIO_OKAY);

  mp_int fR;
  mp_int gR;
  mp_int hR;

  mp_init (&fR);
  mp_init (&gR);
  mp_init (&hR);

  mp_addmod (&pA->f0_share, &pB->f0_share, &cfg->modulus, &fR);
  mp_addmod (&pA->g0_share, &pB->g0_share, &cfg->modulus, &gR);
  mp_addmod (&pA->h0_share, &pB->h0_share, &cfg->modulus, &hR);

  mp_mulmod (&fR, &gR, &cfg->modulus, &fR);
  mu_check (mp_cmp (&fR, &hR) == 0);

  ServerSharedSecret sec = { 
    0x12, 0x87, 0xd1, 0x12, 0x12, 
    0x12, 0x87, 0xd1, 0x32, 0x22, 
    0x12, 0x87, 0xd1, 0x18, 0x84, 
    0x12, 0x87, 0xd1, 0x12, 0x12 };
  PrioVerifier vA = PrioVerifier_new (sA, pA, sec);
  PrioVerifier vB = PrioVerifier_new (sB, pB, sec);
  mu_ensure (vA);
  mu_ensure (vB);

  mp_addmod (&vA->share_fR, &vB->share_fR, &cfg->modulus, &fR);
  mp_addmod (&vA->share_gR, &vB->share_gR, &cfg->modulus, &gR);
  mp_addmod (&vA->share_hR, &vB->share_hR, &cfg->modulus, &hR);

  mp_mulmod (&fR, &gR, &cfg->modulus, &fR);

  //puts ("fR");
  //mp_print (&fR, stdout);
  //puts ("hR");
  //mp_print (&hR, stdout);
  mu_check (mp_cmp (&fR, &hR) == 0);

  mp_clear (&fR);
  mp_clear (&gR);
  mp_clear (&hR);

  PrioVerifier_clear (vA);
  PrioVerifier_clear (vB);

  PrioPacketClient_clear (pA);
  PrioPacketClient_clear (pB);

  PrioServer_clear (sA);
  PrioServer_clear (sB);
  PrioConfig_clear (cfg);
}

