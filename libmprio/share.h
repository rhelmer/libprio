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


#ifndef __SHARE_H__
#define __SHARE_H__

#include "libmpi/mpi.h"
#include "config.h"

struct beaver_triple {
  mp_int a;
  mp_int b;
  mp_int c;
};

typedef struct beaver_triple *BeaverTriple;
typedef const struct beaver_triple *const_BeaverTriple;


/*
 * Use secret sharing to split the int src into two shares.
 * The mp_ints must be initialized.
 */
int share_int (const struct prio_config *cfg, const mp_int *src, 
    mp_int *shareA, mp_int *shareB);

BeaverTriple BeaverTriple_new (void);
void BeaverTriple_clear (BeaverTriple t);

int BeaverTriple_set_rand(const_PrioConfig cfg, 
    BeaverTriple triple_a, 
    BeaverTriple triple_b);

#endif /* __SHARE_H__ */

