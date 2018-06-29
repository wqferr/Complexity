#ifndef INTERPOLATION_H
#define INTERPOLATION_H 1

#include <complex.h>

double lerp(double x0, double x1, double t);
double complex clerp(double complex z0, double complex z1, double t);
double complex clerp_spiral(double complex z0, double complex z1, double t);

#endif