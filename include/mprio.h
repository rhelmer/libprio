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

/* The probability that a cheating client gets caught
 * is roughly:
 *     1 - 2*num_data_fields * 2^{-8*SOUNDNESS_PARAM}, 
 * where num_data_fields is the size of each client packet.
 * Setting this value to 20 is very conservative.
 */
#define SOUNDNESS_PARAM 20

#define PRIO_OKAY 0
#define PRIO_ERROR 1

typedef const unsigned char ServerSharedSecret[SOUNDNESS_PARAM];

/*
 * Opaque types
 */
typedef struct prio_config *PrioConfig;
typedef const struct prio_config *const_PrioConfig;

typedef struct prio_server *PrioServer;
typedef const struct prio_server *const_PrioServer;

typedef struct prio_packet_client *PrioPacketClient;
typedef const struct prio_packet_client *const_PrioPacketClient;

typedef struct prio_total_share *PrioTotalShare;
typedef const struct prio_total_share *const_PrioTotalShare;

typedef struct prio_verifier *PrioVerifier;
typedef const struct prio_verifier *const_PrioVerifier;

typedef struct prio_packet_verify1 *PrioPacketVerify1;
typedef const struct prio_packet_verify1 *const_PrioPacketVerify1;

typedef struct prio_packet_verify2 *PrioPacketVerify2;
typedef const struct prio_packet_verify2 *const_PrioPacketVerify2;


PrioConfig PrioConfig_defaultNew (void);
int PrioConfig_numDataFields (const_PrioConfig cfg);
void PrioConfig_clear (PrioConfig cfg);

int PrioPacketClient_new (const_PrioConfig cfg, const bool *data_in,
    PrioPacketClient *for_server_a, PrioPacketClient *for_server_b);
void PrioPacketClient_clear (PrioPacketClient p);

PrioServer PrioServer_new (const_PrioConfig cfg, int server_idx);
void PrioServer_clear (PrioServer s);

int PrioServer_aggregate (PrioServer s, const_PrioPacketClient p);

PrioTotalShare PrioTotalShare_new (const_PrioServer s);
void PrioTotalShare_clear (PrioTotalShare t);

// Output must have enough space to store a vector with one entry
// per data field.
int PrioTotalShare_final (const_PrioConfig cfg, unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB);

// Don't destroy p until after verification is done.
// shared_secret is a secret value shared between the two 
// verifying servers.
PrioVerifier PrioVerifier_new (PrioServer s, const_PrioPacketClient p,
    ServerSharedSecret secret);
void PrioVerifier_clear (PrioVerifier v);

PrioPacketVerify1 PrioVerifier_packet1 (const_PrioVerifier v);
void PrioPacketVerify1_clear (PrioPacketVerify1 p);

PrioPacketVerify2 PrioVerifier_packet2 (const_PrioVerifier v,
    const_PrioPacketVerify1 pA, const_PrioPacketVerify1 pB);
int PrioVerifier_isValid (const_PrioVerifier v,
    const_PrioPacketVerify2 pA, const_PrioPacketVerify2 pB);
void PrioPacketVerify2_clear (PrioPacketVerify2 p);

#endif /* __PRIO_H__ */

