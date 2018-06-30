#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <omp.h>
#include <math.h>
#include <complex.h>

#include "util/pi.h"

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

#ifndef N_ROOTS
#define N_ROOTS 10
#endif


void init_input(rgba_image **input);
void init_output(rgba_image **output);
void do_stuff(
	const rgba_image *input,
	rgba_image *output,
	const char *path,
	const void *arg);
rgba_image *create_frame(double progress, const void *arg);

double complex f0(double complex z);
double complex f1(double complex z);
void complex_to_hue(rgba_pixel *out, double complex z, const void *arg);
void clean_dir(const char *path);


int main(int argc, char *const argv[]) {
	rgba_image *input;
	rgba_image *output;
	int opt;
	int framerate = DEFAULT_OUTPUT_FPS;
	int duration = 3;
	size_t n_frames;
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

	n_frames = framerate * duration;
	do_stuff(input, output, FRAMES_OUTPUT_DIR, &n_frames);

	rgbaimg_destroy(output);
	rgbaimg_destroy(input);
	free(outfilepath);
	free(outfilename);

	return 0;
}


void init_input(rgba_image **input) {
	*input = NULL;
}


void init_output(rgba_image **output) {
	*output = NULL;
}


void do_stuff(
		const rgba_image *input,
		rgba_image *output,
		const char *path,
		const void *arg) {
	size_t n_frames = *((size_t *) arg);

	(void) input;
	(void) output;

	clean_dir(path);
	animate(
		create_frame, NULL,
		anim_time_smootheststep,
		n_frames, path);
}


double complex lerp_to_cube(double complex z, const void *arg) {
	double t = *((const double *) arg);
	double angle = lerp(0, 2*M_PI / N_ROOTS, t);
	double exponent = lerp(0, 1, angle / (M_PI / 2));
	double complex fz = 1;
	int i;

	for (i = 0; i < N_ROOTS; i++) {
		fz *= (1 - z*cpow(1.0i, -i * exponent));
	}

	return fz;
}


rgba_image *create_frame(double progress, const void *arg) {
	rgba_image *frame = rgbaimg_create(512, 512);
	(void) arg;
	imprint_function_as_hsv(
		frame,
		-2-2.0i, +2+2.0i,
		&lerp_to_cube,
		&progress);
	
	return frame;
}


void clean_dir(const char *path) {
	char *cmd;
	sprintf_alloc(&cmd, "rm -f %s/* --preserve-root", path);
	system(cmd);
	free(cmd);
}