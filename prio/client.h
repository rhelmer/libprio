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


#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "mparray.h"
#include "prg.h"
#include "share.h"


struct server_a_data {
  // These values are only set for server A.
  MPArray data_shares;
  MPArray h_points;
};

struct server_b_data {
  // This value is only used for server B.
  //
  // We use a pseudo-random generator to compress the secret-shared data
  // values. See Appendix I of the Prio paper (the paragraph starting
  // "Optimization: PRG secret sharing.") for details on this.
  PRGSeed seed;
};

/*
 * The data that a Prio client sends to each server. 
 */
struct prio_packet_client {
  // TODO: Can also use a PRG to avoid need for sending Beaver triple shares.
  // Since this optimization only saves ~30 bytes of communication, we haven't
  // bothered implementing it yet.
  BeaverTriple triple;

  mp_int f0_share, g0_share, h0_share;
  ServerId for_server;

  union {
    struct server_a_data A;
    struct server_b_data B;
  } shares;
};


#endif /* __CLIENT_H__ */

