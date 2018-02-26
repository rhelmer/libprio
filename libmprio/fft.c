
#include "config.h"
#include "fft.h"

static void
fft_recurse (mp_int *out, const mp_int *mod, int n, 
    const mp_int *roots, const mp_int *ys,
    mp_int *tmp, mp_int *ySub, mp_int *rootsSub)
{
  if (n == 1) {
    mp_copy (&ys[0], &out[0]);
    return;
  }

  // Recurse on the first half 
  for (int i=0; i<n/2; i++) {
    mp_addmod (&ys[i], &ys[i+(n/2)], mod, &ySub[i]);

    mp_copy (&roots[2*i], &rootsSub[i]);
  }

  fft_recurse (tmp, mod, n/2, rootsSub, ySub, &tmp[n/2], &ySub[n/2], &rootsSub[n/2]);
  for (int i=0; i<n/2; i++) {
    mp_copy (&tmp[i], &out[2*i]);
  }

  // Recurse on the second half 
  for (int i=0; i<n/2; i++) {
    mp_submod (&ys[i], &ys[i+(n/2)], mod, &ySub[i]);
    mp_mulmod (&ySub[i], &roots[i], mod, &ySub[i]);
  }

  fft_recurse (tmp, mod, n/2, rootsSub, ySub, &tmp[n/2], &ySub[n/2], &rootsSub[n/2]);
  for (int i=0; i<n/2; i++) {
    mp_copy (&tmp[i], &out[2*i + 1]);
  }
}

static void 
fft_interpolate_raw (mp_int *out, 
    const mp_int *ys, int nPoints, const mp_int *roots, 
    const mp_int *mod, bool invert)
{
  mp_int tmp[nPoints];
  mp_int ySub[nPoints];
  mp_int rootsSub[nPoints];
  for (int i=0; i<nPoints;i++) {
    mp_init (&tmp[i]);
    mp_init (&ySub[i]);
    mp_init (&rootsSub[i]);
  }

  fft_recurse(out, mod, nPoints, roots, ys, tmp, ySub, rootsSub);

  if (invert) {
    mp_int n_inverse;
    mp_init (&n_inverse);
    mp_set (&n_inverse, nPoints);
    mp_invmod (&n_inverse, mod, &n_inverse);
    for (int i=0; i<nPoints;i++) {
      mp_mulmod(&out[i], &n_inverse, mod, &out[i]);
    }
    mp_clear (&n_inverse); 
  }

  for (int i=0; i<nPoints;i++) {
    mp_clear(&tmp[i]);
    mp_clear(&ySub[i]);
    mp_clear(&rootsSub[i]);
  }
} 

void
fft_get_roots (mp_int *roots_out, int n_points, const_PrioConfig cfg, bool invert)
{
  const mp_int *roots_in = invert ? cfg->rootsInv.data : cfg->roots.data;
  const int step_size = cfg->n_roots / n_points;

  for (int i=0; i < n_points; i++) {
    roots_out[i] = roots_in[i * step_size];
  }
}

int
fft(struct mparray *points_out, const struct mparray *points_in, 
    const_PrioConfig cfg, bool invert)
{
  if (points_out->len != points_in->len)
    return PRIO_ERROR;

  const int n_points = points_in->len;
  if (cfg->n_roots % n_points != 0) 
    return PRIO_ERROR;

  mp_int scaled_roots[n_points];
  fft_get_roots (scaled_roots, n_points, cfg, invert);

  fft_interpolate_raw (points_out->data, points_in->data, n_points, 
      scaled_roots, &cfg->modulus, invert);

  return PRIO_OKAY;
}


