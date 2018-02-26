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


#ifndef __SERVER_H__
#define __SERVER_H__

#include "mparray.h"
#include "share.h"

struct prio_total_share {
  struct mparray data_shares;
};

struct prio_server {
  const_PrioConfig cfg;
  struct mparray data_shares;
};

struct prio_verifier {
  const_PrioConfig cfg;
  const_PrioPacketClient c_packet;

  mp_int share_f0;
  mp_int share_g0;
  mp_int share_h0;
  mp_int share_out;
};

struct prio_packet_verify1 {

};

struct prio_packet_verify2 {

};

#endif /* __SERVER_H__ */

