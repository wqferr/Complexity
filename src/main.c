#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <omp.h>
#include <math.h>
#include <complex.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#include "img/rgbaimg.h"
#include "img/pngio.h"
#include "img/hsv.h"

#include "core/cfunction.h"

#include "anim/anim.h"

#include "util/interpolation.h"
#include "util/misc.h"
#include "util/imprint.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_SUFFIX ".png"

#define FRAMES_OUTPUT_DIR "img/out/"
#define DEFAULT_OUTPUT_FPS 60
#define DEFAULT_OUT_FILE_NAME ("out" IMG_SUFFIX)


void init_input(rgba_image **input);
void init_output(rgba_image **output);
void do_stuff(const rgba_image *input, rgba_image *output);
rgba_image *create_frame(double progress, const void *arg);

double complex f(double complex z);
void complex_to_hue(rgba_pixel *out, double complex z, const void *arg);


int main(int argc, char *const argv[]) {
	rgba_image *input;
	rgba_image *output;
	int opt;
	int framerate = 0;
	int duration = 0;
	char *outfilename = NULL;
	char *outfilepath = NULL;
	(void) framerate;
	(void) duration;

	while ((opt = getopt(argc, argv, "f:t:o:")) != -1) {
		switch (opt) {
			/*
			TODO implement fps control
			case 'f':
				sscanf(optarg, "%d", &framerate);
				if (framerate <= 0) {
					fprintf(stderr, "Framerate must be positive.");
					abort();
				}
				break;
			*/
			case 't':
				sscanf(optarg, "%d", &duration);
				if (duration <= 0) {
					fprintf(stderr, "Duration must be positive.");
					abort();
				}
				break;
			case 'o':
				outfilename = strdup(optarg);
				break;
		}
	}
	if (outfilename == NULL) {
		outfilename = strdup(DEFAULT_OUT_FILE_NAME);
	}
	sprintf_alloc(&outfilepath, "%s%s", IMG_PATH_PREFIX, outfilename);

	init_input(&input);
	init_output(&output);

	do_stuff(input, output);
	png_save_to_file(output, outfilepath);

	rgbaimg_destroy(output);
	rgbaimg_destroy(input);
	free(outfilepath);
	free(outfilename);

	return 0;
}


void init_input(rgba_image **input) {
	*input = rgbaimg_create(512, 512);
}


void init_output(rgba_image **output) {
	*output = rgbaimg_create(512, 512);
}


void do_stuff(const rgba_image *input, rgba_image *output) {
	(void) input;
	imprint_ext(
		output,
		-2-2.0i, +2+2.0i,
		&complex_to_hue, NULL);
}


double complex f(double complex z) {
	return 1.0 - z*z*z;
}


void complex_to_hue(rgba_pixel *out, double complex z, const void *arg) {
	float h, s, v;
	z = f(z);
	h = 360.0f * (carg(z) / (2*M_PI));
	h = fmodf(h + 360.0f, 360.0f);
	s = 1.0f;
	v = 1-exp(-cabs(z));

	hsv_to_rgb(h, s, v, out);
	out->a = 255;
}


rgba_image *create_frame(double progress, const void *arg) {
	(void) arg;
	return NULL;
}


void clean_dir(const char *path) {
	char *cmd;
	sprintf_alloc(&cmd, "rm -f %s/* --preserve-root", path);
	system(cmd);
	free(cmd);
}