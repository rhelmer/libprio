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

#include "libmpi/mpi.h"
#include "include/prio.h"

#include "triple.h"


int 
prio_client_packet_new (const struct prio_config *cfg, const bool *data_in,
    struct prio_packet_client *for_server_a, struct prio_packet_client *for_server_b)
{
  int error;

  if ((error = triple_new (for_server_a->triple)) != PRIO_OKAY)
    return error;
  if ((error = triple_new (for_server_b->triple)) != PRIO_OKAY)
    return error;

  triple_rand (cfg, for_server_a->triple, for_server_b->triple);

  if(data_in[0]) {
  }

  return 0;
}

void
prio_client_packet_clear (struct prio_packet_client *p)
{
  triple_clear (p->triple);
}

