#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <omp.h>
#include <math.h>

#include "img/rgbaimg.h"
#include "img/pngio.h"

#include "core/transform.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

double complex interp_square(double complex z, const void *arg) {
	float t = *((float *) arg);
	return t * cpow(z, 2) + (1-t) * z;
}

rgba_image *read_input_img(int argc, char *const argv[]);
void save_frame(rgba_image *frame, size_t n);

int main(int argc, char *const argv[]) {
	rgba_image *in_img;
	rgba_image *out_img;
	float interp_time, interp_time_step;
	size_t n_frames, cur_frame;
	size_t inwidth, inheight;
	size_t outwidth, outheight;

	in_img = read_input_img(argc, argv);
	rgbaimg_get_dimensions(in_img, &inwidth, &inheight);
	if (getopt(argc, argv, "n:") != -1) {
		n_frames = atoi(optarg);
	}

	interp_time = 0.0f;
	interp_time_step = 1.0f / (n_frames-1);

	outwidth = inwidth;
	outheight = inheight;

	omp_set_nested(true);
	
	system("rm -f " IMG_PATH_PREFIX "out/*" IMG_PATH_SUFFIX);

	#pragma omp parallel for private(out_img, interp_time) shared(in_img)
	for (cur_frame = 0; cur_frame < n_frames; cur_frame++) {
		fprintf(stderr, "Processing frame %zu\n", cur_frame);
		interp_time = cur_frame * interp_time_step;
		out_img = warp_ext(
			in_img, &interp_square, &interp_time,
			(-1-1i), (1+1i),
			(-1-1i), (1+1i),
			outwidth, outheight);
		save_frame(out_img, cur_frame);
		rgbaimg_destroy(out_img);
	}

	rgbaimg_destroy(in_img);
	return 0;
}


rgba_image *read_input_img(int argc, char *const argv[]) {
	char *imgid;
	char *path;
	size_t pathsize;
	rgba_image *input;

	if (getopt(argc, argv, "i:") != -1) {
		imgid = strdup(optarg);
	}

	pathsize = snprintf(NULL, 0, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	path = malloc((pathsize + 1) * sizeof(*path));
	sprintf(path, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	png_load_from_file(&input, path);

	free(imgid);
	free(path);
	return input;
}


void save_frame(rgba_image *frame, size_t n) {
	char *path;
	size_t pathsize;

	pathsize = snprintf(NULL, 0, "%sout/%03zu%s", IMG_PATH_PREFIX, n, IMG_PATH_SUFFIX);
	path = malloc((pathsize + 1) * sizeof(*path));
	sprintf(path, "%sout/%03zu%s", IMG_PATH_PREFIX, n, IMG_PATH_SUFFIX);
	png_save_to_file(frame, path);
	free(path);
}