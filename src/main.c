#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

#include "core/transform.h"

double complex f(double complex z) {
	return ccos(ccos(ccos(z)));
}

int main(int argc, char const *argv[]) {
	rgba_image *in_img;
	rgba_image *out_img;
	double wtime;

	png_load_from_file(&in_img, "img/5_big.png");
	wtime = omp_get_wtime();
	out_img = warp_ext(
		in_img, &f,
		0+0j, 2*M_PI*(1+1j),
		-2-2j, 2+2j,
		0, 0);
	wtime = omp_get_wtime() - wtime;
	printf("%.2lf\n", wtime);
	png_save_to_file(out_img, "img/out.png");
	rgbaimg_destroy(in_img);
	rgbaimg_destroy(out_img);

	return 0;
}