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
	// imprint_ext(
	// 	output,
	// 	-2-2.0i, +2+2.0i,
	// 	&complex_to_hue, NULL);
	// png_save_to_file(output, path);
}


double complex f0(double complex z) {
	return z;
}

double complex f1(double complex z) {
	return 1.0 - z*z*z;
}


void complex_to_hue(rgba_pixel *out, double complex z, const void *arg) {
	float h, s, v;
	double t = *((const double *) arg);
	double angle1 = lerp(0, 2*M_PI/3, t);
	double angle2 = lerp(0, 4*M_PI/3, t);
	double exponent1 = lerp(0, 1, angle1 / (M_PI / 2));
	double exponent2 = lerp(0, 1, angle2 / (M_PI / 2));
	double complex fz = (1+z) * (1 + z*cpow(1.0i, exponent1));
	fz *= (1 + z*cpow(1.0i, exponent2));

	// double p = lerp(1, 3, t);
	// z = 1 + cpow(z, p);
	// z = clerp_spiral(f0(z), f1(z), t);
	h = 360.0f * (carg(fz) / (2*M_PI));
	h = fmodf(h + 360.0f, 360.0f);
	s = 1.0f;
	v = 1-exp(-cabs(fz));

	hsv_to_rgb(h, s, v, out);
	out->a = 255;
}


rgba_image *create_frame(double progress, const void *arg) {
	rgba_image *frame = rgbaimg_create(512, 512);
	(void) arg;
	imprint_ext(
		frame,
		-2-2.0i, +2+2.0i,
		&complex_to_hue,
		&progress);
	
	return frame;
}


void clean_dir(const char *path) {
	char *cmd;
	sprintf_alloc(&cmd, "rm -f %s/* --preserve-root", path);
	system(cmd);
	free(cmd);
}