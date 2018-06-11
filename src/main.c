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

#include "core/cfunction.h"

#include "util/interpolation.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

double complex f(double complex z, const void *arg) {
	float t = *((float *) arg);
	return clerp(z, ccos(z), t);
}

rgba_image *read_input_img(int argc, char *const argv[]);
void save_frame(rgba_image *frame, size_t n);
float time(size_t frame, size_t n_frames);
float sigmoid(float t);

int main(int argc, char *const argv[]) {
	rgba_image *in_img;
	rgba_image *out_img;
	float interp_time;
	size_t n_frames, cur_frame;
	size_t inwidth, inheight;
	size_t outwidth, outheight;

	in_img = read_input_img(argc, argv);
	rgbaimg_get_dimensions(in_img, &inwidth, &inheight);
	if (getopt(argc, argv, "n:") != -1) {
		n_frames = atoi(optarg);
	}

	outwidth = inwidth;
	outheight = inheight;

	omp_set_nested(true);
	
	system("rm -f " IMG_PATH_PREFIX "out/*" IMG_PATH_SUFFIX);

	#pragma omp parallel for\
		num_threads(8)\
		private(out_img, interp_time)\
		shared(in_img)
	for (cur_frame = 0; cur_frame < n_frames; cur_frame++) {
		fprintf(stderr, "Processing frame %zu\n", cur_frame);
		interp_time = time(cur_frame, n_frames);
		out_img = warp_ext(
			in_img, &f, &interp_time,
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


#define STEEPNESS 1
#define SIGMOID_START -10
#define SIGMOID_END 15
float time(size_t frame, size_t n_frames) {
	float t;
	if (frame == 0) {
		return 0;
	}
	t = lerp(SIGMOID_START, SIGMOID_END, (double) frame / n_frames);
	// t = (float) frame / n_frames;
	// t = t * (SIGMOID_END - SIGMOID_START) + SIGMOID_START;
	return 1.0f / (1 + exp(STEEPNESS * (-t)));
}