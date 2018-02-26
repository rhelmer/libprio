#ifndef _FFT__H
#define _FFT__H

#include <mprio.h>
#include <stdbool.h>

#include "libmpi/mpi.h"
#include "mparray.h"

// TOOD: Add error handling
int fft(struct mparray *points_out, 
    const struct mparray *points_in, const_PrioConfig cfg, bool invert);

// Get an array
//   (r^0, r^1, r^2, ... )
// where r is an n-th root of unity.
// 
// Do NOT mp_clear() the mp_ints stored in roots_out. 
// These are owned by the PrioConfig object.
void fft_get_roots (mp_int *roots_out, int n_points,
    const_PrioConfig cfg, bool invert);

#endif
