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


/*
 * Use secret sharing to split the int src into two shares.
 */
int share_int (const struct prio_config *cfg, const mp_int *src, 
    mp_int *shareA, mp_int *shareB);

int triple_new (struct beaver_triple *triple);
void triple_clear (struct beaver_triple *triple);

int triple_rand (const struct prio_config *cfg, 
    struct beaver_triple *triple_a, 
    struct beaver_triple *triple_b);

#endif /* __SHARE_H__ */

