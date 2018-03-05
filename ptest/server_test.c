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

#include "mpi/mpi.h"
#include "prio/server.h"
#include "prio/server.c"

void mu_test__eval_poly (void) 
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  MPArray coeffs = NULL;
  mp_int eval_at, out;

  MP_DIGITS (&eval_at) = NULL;
  MP_DIGITS (&out) = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (coeffs = MPArray_new (3));

  mp_set (&coeffs->data[0], 2);
  mp_set (&coeffs->data[1], 8);
  mp_set (&coeffs->data[2], 3);

  MP_CHECKC (mp_init (&eval_at));
  MP_CHECKC (mp_init (&out));
  mp_set (&eval_at, 7);

  const int val = 3*7*7 + 8*7 + 2;
  mu_check (poly_eval (&out, coeffs, &eval_at, cfg) == SECSuccess);
  mu_check (mp_cmp_d (&out, val) == 0);

cleanup:
  mu_check (rv == SECSuccess);
  mp_clear (&out);
  mp_clear (&eval_at);
  MPArray_clear (coeffs);
  PrioConfig_clear (cfg);
}

void 
mu_test__verify_new (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  PrioServer sA = NULL;
  PrioServer sB = NULL;
  PrioPacketClient pA = NULL;
  PrioPacketClient pB = NULL;
  PrioVerifier vA = NULL;
  PrioVerifier vB = NULL;

  mp_int fR, gR, hR;
  MP_DIGITS (&fR) = NULL;
  MP_DIGITS (&gR) = NULL;
  MP_DIGITS (&hR) = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());

  const int ndata = PrioConfig_numDataFields (cfg);
  {
  bool data_items[ndata];
  for (int i=0; i < ndata; i++) {
    // Arbitrary data
    data_items[i] = (i % 3 == 1) || (i % 5 == 3);
  }

  P_CHECKA (sA = PrioServer_new (cfg, 0));
  P_CHECKA (sB = PrioServer_new (cfg, 1));

  P_CHECKA (pA = PrioPacketClient_new (cfg, PRIO_SERVER_A));
  P_CHECKA (pB = PrioPacketClient_new (cfg, PRIO_SERVER_B));

  P_CHECKC (PrioPacketClient_set_data (cfg, data_items, pA, pB));

  MP_CHECKC (mp_init (&fR));
  MP_CHECKC (mp_init (&gR));
  MP_CHECKC (mp_init (&hR));

  MP_CHECKC (mp_addmod (&pA->f0_share, &pB->f0_share, &cfg->modulus, &fR));
  MP_CHECKC (mp_addmod (&pA->g0_share, &pB->g0_share, &cfg->modulus, &gR));
  MP_CHECKC (mp_addmod (&pA->h0_share, &pB->h0_share, &cfg->modulus, &hR));

  MP_CHECKC (mp_mulmod (&fR, &gR, &cfg->modulus, &fR));
  mu_check (mp_cmp (&fR, &hR) == 0);

  ServerSharedSecret sec = { 
    0x12, 0x87, 0xd1, 0x12, 0x12, 
    0x12, 0x87, 0xd1, 0x32, 0x22, 
    0x12, 0x87, 0xd1, 0x18, 0x84, 
    0x12, 0x87, 0xd1, 0x12, 0x12 };
  P_CHECKA (vA = PrioVerifier_new (sA));
  P_CHECKA (vB = PrioVerifier_new (sB));
  P_CHECKC (PrioVerifier_set_data (vA, pA, sec));
  P_CHECKC (PrioVerifier_set_data (vB, pB, sec));

  MP_CHECKC (mp_addmod (&vA->share_fR, &vB->share_fR, &cfg->modulus, &fR));
  MP_CHECKC (mp_addmod (&vA->share_gR, &vB->share_gR, &cfg->modulus, &gR));
  MP_CHECKC (mp_addmod (&vA->share_hR, &vB->share_hR, &cfg->modulus, &hR));

  MP_CHECKC (mp_mulmod (&fR, &gR, &cfg->modulus, &fR));

  //puts ("fR");
  //mp_print (&fR, stdout);
  //puts ("hR");
  //mp_print (&hR, stdout);
  mu_check (mp_cmp (&fR, &hR) == 0);
  }

cleanup:
  mu_check (rv == SECSuccess);

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

void
verify_full (int tweak)
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

  mp_int fR, gR, hR;
  MP_DIGITS (&fR) = NULL;
  MP_DIGITS (&gR) = NULL;
  MP_DIGITS (&hR) = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());

  const int ndata = PrioConfig_numDataFields (cfg);
  {
  bool data_items[ndata];
  for (int i=0; i < ndata; i++) {
    // Arbitrary data
    data_items[i] = (i % 3 == 1) || (i % 5 == 3);
  }

  P_CHECKA (sA = PrioServer_new (cfg, 0));
  P_CHECKA (sB = PrioServer_new (cfg, 1));

  P_CHECKA (pA = PrioPacketClient_new (cfg, PRIO_SERVER_A));
  P_CHECKA (pB = PrioPacketClient_new (cfg, PRIO_SERVER_B));

  P_CHECKC (PrioPacketClient_set_data (cfg, data_items, pA, pB));
  if (tweak == 3) {
    mp_add_d (&pA->shares.A.h_points->data[1], 1, 
        &pA->shares.A.h_points->data[1]);
  }

  if (tweak == 4) {
    mp_add_d (&pA->shares.A.data_shares->data[1], 1, 
        &pA->shares.A.data_shares->data[1]);
  }

  ServerSharedSecret sec = { 
    0x12, 0x87, 0xd1, 0x12, 0x12, 
    0x12, 0x87, 0xd1, 0x32, 0x22, 
    0x12, 0x87, 0xd1, 0x18, 0x84, 
    0x12, 0x87, 0xd1, 0x12, 0x12 };
  P_CHECKA (vA = PrioVerifier_new (sA));
  P_CHECKA (vB = PrioVerifier_new (sB));
  P_CHECKC (PrioVerifier_set_data (vA, pA, sec));
  P_CHECKC (PrioVerifier_set_data (vB, pB, sec));

  P_CHECKA (p1A = PrioPacketVerify1_new ());
  P_CHECKA (p1B = PrioPacketVerify1_new ());

  P_CHECKC (PrioPacketVerify1_set_data (p1A, vA));
  P_CHECKC (PrioPacketVerify1_set_data (p1B, vB));

  if (tweak == 1) {
    mp_add_d (&p1B->share_d, 1, &p1B->share_d);
  }

  P_CHECKA (p2A = PrioPacketVerify2_new ());
  P_CHECKA (p2B = PrioPacketVerify2_new ());
  P_CHECKC (PrioPacketVerify2_set_data (p2A, vA, p1A, p1B));
  P_CHECKC (PrioPacketVerify2_set_data (p2B, vB, p1A, p1B));

  if (tweak == 2) {
    mp_add_d (&p2A->share_out, 1, &p2B->share_out);
  }

  int shouldBe = tweak ? SECFailure : SECSuccess;
  mu_check (PrioVerifier_isValid (vA, p2A, p2B) == shouldBe);
  mu_check (PrioVerifier_isValid (vB, p2A, p2B) == shouldBe);
  }

cleanup:
  if (!tweak) {
    mu_check (rv == SECSuccess);
  }
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

void 
mu_test__verify_full_good (void)
{
  verify_full (0);
}

void 
mu_test__verify_full_bad1 (void)
{
  verify_full (1);
}

void 
mu_test__verify_full_bad2 (void)
{
  verify_full (2);
}

void 
mu_test__verify_full_bad3 (void)
{
  verify_full (3);
}

void 
mu_test__verify_full_bad4 (void)
{
  verify_full (4);
}

