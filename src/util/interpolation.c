#include "util/interpolation.h"

#include <math.h>

#include "util/pi.h"


double lerp(double x0, double x1, double t) {
    return (1-t)*x0 + t*x1;
}


double complex clerp(double complex z0, double complex z1, double t) {
    return (1-t)*z0 + t*z1;
}

double complex clerp_spiral(double complex z0, double complex z1, double t) {
    double r = lerp(cabs(z0), cabs(z1), t);
    double a0 = carg(z0);
    double a1 = carg(z1);
    double a;
    if (a1 < a0) {
        a1 += 2*M_PI;
    }
    a = lerp(a0, a1, t);
    return r*cos(a) + r*sin(a)*1.0i;
}