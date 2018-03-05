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

#include <nss/blapi.h>
#include <nss/seccomon.h>
#include <stdbool.h>
#include <stddef.h>

/* Seed for a pseudo-random generator (PRG). */
#define PRG_SEED_LENGTH AES_128_KEY_LENGTH
typedef unsigned char PrioPRGSeed[PRG_SEED_LENGTH];

/*
 * Type for each of the two servers.
 */
typedef enum {
  PRIO_SERVER_A,
  PRIO_SERVER_B
} PrioServerId;

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
 * server B. The `for_server` parameter determines which server
 * the packet is for.
 */
PrioPacketClient PrioPacketClient_new (const_PrioConfig cfg, PrioServerId for_server);
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
 * Generate a new PRG seed using the NSS global randomness source.
 * Use this routine to initialize the secret that the two Prio servers
 * share.
 */
SECStatus PrioPRGSeed_randomize (PrioPRGSeed *seed);

/*
 * The PrioServer object holds the state of the Prio servers.
 * Pass in the _same_ secret PRGSeed when initializing the two servers.
 * The PRGSeed must remain secret to the two servers.
 */
PrioServer PrioServer_new (const_PrioConfig cfg, PrioServerId server_idx,
    const PrioPRGSeed server_shared_secret);
void PrioServer_clear (PrioServer s);


/* 
 * After receiving a client packet, each of the servers generate
 * a PrioVerifier object that they use to check whether the client's
 * encoded packet is well formed.
 *
 * Don't clear the packet p until after verification is done.
 */
PrioVerifier PrioVerifier_new (PrioServer s);
void PrioVerifier_clear (PrioVerifier v);

SECStatus PrioVerifier_set_data (PrioVerifier v, const_PrioPacketClient p);

/*
 * Generate the first packet that servers need to exchange
 * to verify the client's submission. This should be sent
 * over a TLS connection between the servers.
 */
PrioPacketVerify1 PrioPacketVerify1_new (void);
void PrioPacketVerify1_clear (PrioPacketVerify1 p1);

SECStatus PrioPacketVerify1_set_data (PrioPacketVerify1 p1,
    const_PrioVerifier v);

/* 
 * Generate the second packet that the servers need to exchange
 * to verify the client's submission. The routine takes as input
 * the PrioPacketVerify1 packets from both server A and server B.
 *
 * This should be sent over a TLS connection between the servers.
 */
PrioPacketVerify2 PrioPacketVerify2_new (void);
void PrioPacketVerify2_clear (PrioPacketVerify2 p);

SECStatus PrioPacketVerify2_set_data (PrioPacketVerify2 p2, const_PrioVerifier v,
    const_PrioPacketVerify1 p1A, const_PrioPacketVerify1 p1B);

/* 
 * Use the PrioPacketVerify2s from both servers to check whether
 * the client's submission is well formed.
 */
SECStatus PrioVerifier_isValid (const_PrioVerifier v,
    const_PrioPacketVerify2 pA, const_PrioPacketVerify2 pB);

/*
 * Each of the two servers calls this routine to aggregate the data
 * submission from a client that is included in the PrioVerifier object.
 *
 * IMPORTANT: This routine does not check the validity of the client's
 * data packet. The servers must execute the verification checks 
 * above before aggregating any client data.
 */
SECStatus PrioServer_aggregate (PrioServer s, PrioVerifier v);

/* 
 * After the servers have aggregated data packets from "enough" clients
 * (this determines the anonymity set size), each server runs this routine
 * to get a share of the aggregate statistics. 
 */
PrioTotalShare PrioTotalShare_new (void);
void PrioTotalShare_clear (PrioTotalShare t);

SECStatus PrioTotalShare_set_data (PrioTotalShare t, const_PrioServer s);

/*
 * Read the output data into an array of unsigned longs. You should
 * be sure that each data value can fit into a single long and that
 * the pointer `output` points to a buffer large enough to store
 * one long per data field.
 */
SECStatus PrioTotalShare_final (const_PrioConfig cfg, unsigned long *output,
    const_PrioTotalShare tA, const_PrioTotalShare tB);


#endif /* __PRIO_H__ */

