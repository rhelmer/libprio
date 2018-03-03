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

#include <nss/seccomon.h>
#include <stdbool.h>
#include <stddef.h>

/* The probability that a cheating client gets caught
 * is roughly:
 *     1 - 2*num_data_fields * 2^{-8*SOUNDNESS_PARAM}, 
 * where num_data_fields is the size of each client packet.
 * Setting this value to 20 is very conservative.
 */
#define SOUNDNESS_PARAM 20

typedef unsigned char ServerSharedSecret[SOUNDNESS_PARAM];

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


/* 
 * Initialize and clear random number generator state.
 * You must call Prio_init() before using the library.
 * To avoid memory leaks, call Prio_clear() afterwards.
 */
SECStatus Prio_init ();
void Prio_clear();

/* 
 * PrioConfig holds the system parameters. The two relevant
 * things determined by the config object are:
 *    (1) the number of data fields we are collecting, and
 *    (2) the modulus we use for modular arithmetic.
 * The default configuration uses an 87-bit modulus.
 */
PrioConfig PrioConfig_new (int n_fields);
PrioConfig PrioConfig_defaultNew (void);
void PrioConfig_clear (PrioConfig cfg);
int PrioConfig_numDataFields (const_PrioConfig cfg);

/*
 * The PrioPacketClient object holds the encoded client data.
 * The client sends one packet to server A and one packet to
 * server B.
 */
PrioPacketClient PrioPacketClient_new (const_PrioConfig cfg);
void PrioPacketClient_clear (PrioPacketClient p);

/*
 *  PrioPacketClient_set_data
 *
 * Takes as input a pointer to an array (`data_in`) of boolean values 
 * whose length is equal to the number of data fields specified in 
 * the config. It then encodes the data into a pair of `PrioPacketClient`s.
 *
 * IMPORTANT TODO: The data in the PrioPacketClient objects MUST
 * BE ENCRYPTED to the public keys of servers A and B. We need to
 * figure out the right NSS routines to use here.
 */
SECStatus PrioPacketClient_set_data (const_PrioConfig cfg, const bool *data_in,
    PrioPacketClient for_server_a, PrioPacketClient for_server_b);

/*
 * The PrioServer object holds the state of the Prio servers.
 * Server A gets server_idx 0 and server B gets server_idx 1.
 */
PrioServer PrioServer_new (const_PrioConfig cfg, int server_idx);
void PrioServer_clear (PrioServer s);


/* 
 * After receiving a client packet, each of the servers generate
 * a PrioVerifier object that they use to check whether the client's
 * encoded packet is well formed.
 *
 * Don't clear the packet p until after verification is done.
 *
 * IMPORTANT: The value shared_secret passed in here is a 
 * secret value shared between the two servers. The servers must
 * use a fresh random value for _each_ verification step.
 * In practice, the servers can share a short seed (e.g., 128 bits)
 * and can use AES in counter mode to generate many shared
 * secrets without interacting further.
 */
PrioVerifier PrioVerifier_new (PrioServer s, const_PrioPacketClient p,
    const ServerSharedSecret secret);
void PrioVerifier_clear (PrioVerifier v);

/*
 * Generate the first packet that servers need to exchange
 * to verify the client's submission. This should be sent
 * over a TLS connection between the servers.
 */
PrioPacketVerify1 PrioVerifier_packet1 (const_PrioVerifier v);
void PrioPacketVerify1_clear (PrioPacketVerify1 p);

/* 
 * Generate the second packet that the servers need to exchange
 * to verify the client's submission. The routine takes as input
 * the PrioPacketVerify1 packets from both server A and server B.
 *
 * This should be sent over a TLS connection between the servers.
 */
PrioPacketVerify2 PrioVerifier_packet2 (const_PrioVerifier v,
    const_PrioPacketVerify1 pA, const_PrioPacketVerify1 pB);
void PrioPacketVerify2_clear (PrioPacketVerify2 p);

/* 
 * Use the PrioPacketVerify2s from both servers to check whether
 * the client's submission is well formed.
 */
SECStatus PrioVerifier_isValid (const_PrioVerifier v,
    const_PrioPacketVerify2 pA, const_PrioPacketVerify2 pB);

/*
 * Each of the two servers calls this routine to aggregate a data
 * submission from a client.
 *
 * IMPORTANT: This routine does not check the validity of the client's
 * data packet. The servers must execute the verification checks 
 * above before aggregating any client data.
 */
SECStatus PrioServer_aggregate (PrioServer s, const_PrioPacketClient p);

/* 
 * After the servers have aggregated data packets from "enough" clients
 * (this determines the anonymity set size), each server runs this routine
 * to get a share of the aggregate statistics. 
 */
PrioTotalShare PrioTotalShare_new (const_PrioServer s);
void PrioTotalShare_clear (PrioTotalShare t);

/*
 * Read the output data into an array of unsigned longs. You should
 * be sure that each data value can fit into a single long and that
 * the pointer `output` points to a buffer large enough to store
 * one long per data field.
 */
SECStatus PrioTotalShare_final (const_PrioConfig cfg, unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB);


#endif /* __PRIO_H__ */

