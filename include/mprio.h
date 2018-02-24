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

#define PRIO_OKAY 0
#define PRIO_ERROR 1

typedef struct prio_config *PrioConfig;
typedef const struct prio_config *const_PrioConfig;

typedef struct prio_server *PrioServer;
typedef const struct prio_server *const_PrioServer;

typedef struct prio_packet_client *PrioPacketClient;
typedef const struct prio_packet_client *const_PrioPacketClient;

typedef struct prio_packet_verify *PrioPacketVerify;
typedef const struct prio_packet_verify *const_PrioPacketVerify;

typedef struct prio_total_share *PrioTotalShare;
typedef const struct prio_total_share *const_PrioTotalShare;


PrioConfig PrioConfig_defaultNew (void);
int PrioConfig_numDataFields (const_PrioConfig cfg);
void PrioConfig_clear (PrioConfig cfg);

int PrioPacketClient_new (const_PrioConfig cfg, const bool *data_in,
    PrioPacketClient *for_server_a, PrioPacketClient *for_server_b);
void PrioPacketClient_clear (PrioPacketClient p);

PrioServer PrioServer_new (const_PrioConfig cfg);
void PrioServer_clear (PrioServer s);

PrioPacketVerify PrioServer_newPacketVerify (const_PrioPacketClient p);
void PrioPacketVerify_clear (PrioPacketVerify p);

int PrioServer_isValid (const_PrioServer s,
    const_PrioPacketVerify pA,
    const_PrioPacketVerify pB);
int PrioServer_aggregate (PrioServer s, const_PrioPacketClient p);

PrioTotalShare PrioServer_newTotalShare (const_PrioServer s);

// Output must have enough space to store a vector with one entry
// per data field.
int PrioTotalShare_final (const_PrioConfig cfg, unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB);

void PrioTotalShare_clear (PrioTotalShare t);



#endif /* __PRIO_H__ */

