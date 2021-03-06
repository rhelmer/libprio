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

#include "mpi/mpi.h"
#include "prio/config.h"
#include "prio/mparray.h"
#include "prio/share.h"
#include "prio/util.h"
#include "mutest.h"

void 
mu_test_share (void)
{
  SECStatus rv = SECSuccess;
  PrioConfig cfg = NULL;
  mp_int a, b, c;
  BeaverTriple t1 = NULL, t2 = NULL;

  MP_DIGITS (&a) = NULL;
  MP_DIGITS (&b) = NULL;
  MP_DIGITS (&c) = NULL;

  P_CHECKA (cfg = PrioConfig_defaultNew());
  P_CHECKA (t1 = BeaverTriple_new ());
  P_CHECKA (t2 = BeaverTriple_new ());

  mu_check (BeaverTriple_set_rand (cfg, t1, t2) == SECSuccess);

  MP_CHECKC (mp_init (&a)); 
  MP_CHECKC (mp_init (&b)); 
  MP_CHECKC (mp_init (&c)); 

  mu_check (mp_addmod (&t1->a, &t2->a, &cfg->modulus, &a) == MP_OKAY);
  mu_check (mp_addmod (&t1->b, &t2->b, &cfg->modulus, &b) == MP_OKAY);
  mu_check (mp_addmod (&t1->c, &t2->c, &cfg->modulus, &c) == MP_OKAY);
  mu_check (mp_mulmod (&a, &b, &cfg->modulus, &a) == MP_OKAY);
  mu_check (mp_cmp (&a, &c) == 0);

cleanup:
  mu_check (rv == SECSuccess);
  mp_clear (&a);
  mp_clear (&b);
  mp_clear (&c);

  PrioConfig_clear (cfg);
  BeaverTriple_clear (t1);
  BeaverTriple_clear (t2);
}

void 
mu_test_arr (void)
{
  SECStatus rv = SECSuccess;
  MPArray arr = NULL;
  MPArray arr2 = NULL;
  P_CHECKA (arr = MPArray_new (10));
  P_CHECKA (arr2 = MPArray_new (7));

  for (int i=0; i<10; i++) {
    mp_set (&arr->data[i], i);
  }

  P_CHECKC (MPArray_resize (arr, 15));
  for (int i=10; i<15; i++) {
    mu_check (mp_cmp_d (&arr->data[i], 0) == 0);
    mp_set (&arr->data[i], i);
  }

  P_CHECKC (MPArray_resize (arr, 7));
  for (int i=10; i<7; i++) {
    mu_check (mp_cmp_d (&arr->data[i], i) == 0);
  }

  P_CHECKC (MPArray_copy (arr2, arr));
  for (int i=10; i<7; i++) {
    mu_check (mp_cmp (&arr->data[i], &arr2->data[i]) == 0);
  }

cleanup:
  mu_check (rv == SECSuccess);
  MPArray_clear (arr);
  MPArray_clear (arr2);
}
