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


#include <mprio.h>

#include "libmpi/mpi.h"
#include "libmprio/config.h"
#include "libmprio/mparray.h"
#include "libmprio/share.h"
#include "mutest.h"

void 
mu_test_share (void)
{
  struct beaver_triple t1, t2;
  mu_check (triple_new (&t1) == SECSuccess);
  mu_check (triple_new (&t2) == SECSuccess);

  PrioConfig cfg = PrioConfig_defaultNew();
  mu_check (cfg);

  mu_check (triple_rand (cfg, &t1, &t2) == SECSuccess);

  mp_int a, b, c;
  mu_check (mp_init (&a) == MP_OKAY);
  mu_check (mp_init (&b) == MP_OKAY);
  mu_check (mp_init (&c) == MP_OKAY);

  mu_check (mp_addmod (&t1.a, &t2.a, &cfg->modulus, &a) == MP_OKAY);
  mu_check (mp_addmod (&t1.b, &t2.b, &cfg->modulus, &b) == MP_OKAY);
  mu_check (mp_addmod (&t1.c, &t2.c, &cfg->modulus, &c) == MP_OKAY);
  mu_check (mp_mulmod (&a, &b, &cfg->modulus, &a) == MP_OKAY);
  mu_check (mp_cmp (&a, &c) == 0);

  mp_clear (&a);
  mp_clear (&b);
  mp_clear (&c);

  PrioConfig_clear (cfg);
  triple_clear (&t1);
  triple_clear (&t2);
}

void 
mu_test_arr (void)
{
  MPArray arr = MPArray_init (10);
  mu_ensure (arr);

  for (int i=0; i<10; i++) {
    mp_set (&arr->data[i], i);
  }

  mu_ensure (MPArray_resize (arr, 15) == SECSuccess);
  for (int i=10; i<15; i++) {
    mu_check (mp_cmp_d (&arr->data[i], 0) == 0);
    mp_set (&arr->data[i], i);
  }

  mu_ensure (MPArray_resize (arr, 7) == SECSuccess);
  for (int i=10; i<7; i++) {
    mu_check (mp_cmp_d (&arr->data[i], i) == 0);
  }

  MPArray_clear (arr);
}
