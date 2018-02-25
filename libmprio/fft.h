#ifndef _FFT__H
#define _FFT__H

#include <mprio.h>
#include <stdbool.h>

#include "libmpi/mpi.h"

// TOOD: Add error handling
int fft_interpolate(mp_int *points_out, const mp_int *points_in, 
    int n_points, const_PrioConfig cfg, bool invert);

// Get an array
//   (r^0, r^1, r^2, ... )
// where r is an n-th root of unity.
void fft_get_roots (mp_int *roots_out, int n_points, const_PrioConfig cfg, bool invert);

#endif
