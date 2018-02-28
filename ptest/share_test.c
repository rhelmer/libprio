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
#include "libmprio/util.h"
#include "mutest.h"

void 
mu_test_share (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  P_CHECKA (cfg = PrioConfig_defaultNew());

  struct beaver_triple t1, t2;
  P_CHECKC (triple_new (&t1));
  P_CHECKC (triple_new (&t2));

  mu_check (triple_rand (cfg, &t1, &t2) == SECSuccess);

  mp_int a, b, c;
  MP_CHECKC (mp_init (&a)); 
  MP_CHECKC (mp_init (&b)); 
  MP_CHECKC (mp_init (&c)); 

  mu_check (mp_addmod (&t1.a, &t2.a, &cfg->modulus, &a) == MP_OKAY);
  mu_check (mp_addmod (&t1.b, &t2.b, &cfg->modulus, &b) == MP_OKAY);
  mu_check (mp_addmod (&t1.c, &t2.c, &cfg->modulus, &c) == MP_OKAY);
  mu_check (mp_mulmod (&a, &b, &cfg->modulus, &a) == MP_OKAY);
  mu_check (mp_cmp (&a, &c) == 0);

cleanup:
  mu_check (rv == SECSuccess);
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
  SECStatus rv = SECSuccess;
  MPArray arr = NULL;
  MP_CHECKC (arr = MPArray_init (10));

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

cleanup:
  mu_check (rv == SECSuccess);
  MPArray_clear (arr);
}
