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
#include "libprio/rand.h"
#include "mutest.h"

void 
test_rand_once (int limit)
{
  mp_int max;
  mp_int out;

  mu_check (mp_init (&max) == MP_OKAY);
  mu_check (mp_init (&out) == MP_OKAY);

  mp_set (&max, limit);

  mu_check (rand_int (&out, &max)  == MP_OKAY);
  mu_check (mp_cmp_d (&out, limit) == -1);
  mu_check (mp_cmp_z (&out) > -1);

  mp_clear (&max);
  mp_clear (&out);
}

void 
mu_test_rand__multiple_of_8 (void) 
{
  test_rand_once (256);
  test_rand_once (256*256);
}


void 
mu_test_rand__near_multiple_of_8 (void) 
{
  test_rand_once (256+1);
  test_rand_once (256*256+1);
}

void 
mu_test_rand__odd (void) 
{
  test_rand_once (39);
  test_rand_once (123);
  test_rand_once (993123);
}

void 
mu_test_rand__large (void) 
{
  test_rand_once (1231239933);
}

void 
mu_test_rand__bit(void) 
{
  test_rand_once (1);
  for (int i = 0; i < 100; i++)
    test_rand_once (2);
}

void 
mu_test_rand__distribution(void) 
{
  const int limit = 123;
  int bins[limit];

  mp_int max;
  mp_int out;

  mu_check (mp_init (&max) == MP_OKAY);
  mu_check (mp_init (&out) == MP_OKAY);

  mp_set (&max, limit);

  for (int i = 0; i < limit; i++) {
    bins[i] = 0;
  }

  for (int i = 0; i < limit*limit; i++) {
    mu_check (rand_int (&out, &max)  == MP_OKAY);
    mu_check (mp_cmp_d (&out, limit) == -1);
    mu_check (mp_cmp_z (&out) > -1);

    unsigned char ival;
    mu_check (mp_to_fixlen_octets (&out, &ival, 1) == MP_OKAY);
    bins[ival] += 1;
  }

  for (int i = 0; i < limit; i++) {
    mu_check (bins[i] > 50);
  }

  mp_clear (&max);
  mp_clear (&out);
}
