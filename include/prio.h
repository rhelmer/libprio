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


#ifndef __PRIO_H__
#define __PRIO_H__

#include <stdbool.h>
#include <stddef.h>

#define ERROR 1

struct prio_config {
  int data_len;
};

struct prio_packet_client {
  int blah;
};

struct prio_packet_server_cor {
  int blah;
};

struct prio_packet_server_out {
  int blah;
};


int prio_client_encode (const struct prio_config *c, const bool data_in[],
    struct prio_packet_client *for_server_a, struct prio_packet_client *for_server_b);

int prio_server_correction (const struct prio_config *c, 
    const struct prio_packet_client *for_server,
    struct prio_packet_server_cor *cor);

int prio_server_valid (const struct prio_config *c, 
    struct prio_packet_server_cor *corA,
    struct prio_packet_server_cor *corB);


#endif /* __PRIO_H__ */

