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


#include "mpi/mpi.h"
#include "mutest.h"


void 
mu_test_mpi__add (void) 
{
  mp_int a;
  mp_int b;
  mp_int c;

  mu_check (mp_init (&a) == MP_OKAY);
  mu_check (mp_init (&b) == MP_OKAY);
  mu_check (mp_init (&c) == MP_OKAY);

  mp_set (&a, 10);
  mp_set (&b, 7);
  mp_add (&a, &b, &c);

  mp_set (&a, 17);
  mu_check (mp_cmp (&a, &c) == 0);

  mp_clear (&a);
  mp_clear (&b);
  mp_clear (&c);
}



