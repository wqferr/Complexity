#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

#include "core/transform.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

double complex f(double complex z, const void *arg) {
	(void) arg;
	return cpow(z, 2);
}

int main(int argc, char const *argv[]) {
	rgba_image *in_img;
	rgba_image *out_img;
	size_t width, height;
	double wtime;
	const char *imgid;
	char *path;

	if (argc > 1) {
		imgid = argv[1];
	} else {
		imgid = "0";
	}
	path = calloc(
		strlen(IMG_PATH_PREFIX)
		+ strlen(imgid)
		+ strlen(IMG_PATH_SUFFIX)
		+ 1,
		sizeof(char));
	strcpy(path, IMG_PATH_PREFIX);
	strcat(path, imgid);
	strcat(path, IMG_PATH_SUFFIX);

	png_load_from_file(&in_img, path);
	rgbaimg_get_dimensions(in_img, &width, &height);
	wtime = omp_get_wtime();
	out_img = warp_ext(
		in_img, &f, NULL,
		(-1-1i), (1+1i),
		(-1-1i), (1+1i),
		width/4, height/4);
	wtime = omp_get_wtime() - wtime;
	printf("%.2lf\n", wtime);
	png_save_to_file(out_img, "img/out.png");
	rgbaimg_destroy(in_img);
	rgbaimg_destroy(out_img);
	free(path);

	return 0;
}