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

#include "anim/anim.h"

#include "util/interpolation.h"
#include "util/misc.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

#define FRAMES_OUTPUT_DIR "img/out/"
#define OUTPUT_FPS 60


double complex cexp_taylor(double complex z, double t) {
	double complex out = clerp(0, 1, t) + z;
	int i, f = 1;
	for (i = 2; i < 10; i++) {
		f *= i;
		out += clerp(0, cpow(z, i)/f, t);
	}
	return out;
}

double complex f(double complex z, const void *arg) {
	double t = *((const double *) arg);
	return cexp_taylor(z, t);
}

rgba_pixel imprint_x2(double complex z, const void *arg) {
	rgba_pixel out = { .r = 0, .g = 0, .b = 0, .a = 255 };
	double margin = *((const double *) arg);
	double x = creal(z);
	double y = cimag(z);

	if (fabs(x*x - y) < margin) {
		out.r = 255;
		out.g = 255;
		out.b = 255;
	}

	return out;
}

void clean_dir(const char *path);

rgba_image *read_input_img(int argc, char *const argv[]);
rgba_image *create_frame(double progress, const void *arg);

int main_test_imprint(int argc, char *const argv[]);
int main_create_warp_anim(int argc, char *const argv[]);

int main(int argc, char *const argv[]) {
	return main_create_warp_anim(argc, argv);
}

int main_create_warp_anim(int argc, char *const argv[]) {
	rgba_image *in_img;
	float out_duration;
	size_t n_frames;
	size_t inwidth, inheight;

	in_img = read_input_img(argc, argv);
	rgbaimg_get_dimensions(in_img, &inwidth, &inheight);
	if (getopt(argc, argv, "t:") != -1) {
		sscanf(optarg, "%f", &out_duration);
		n_frames = out_duration * OUTPUT_FPS;
	}

	omp_set_nested(true);
	
	clean_dir(FRAMES_OUTPUT_DIR);

	animate(
		&create_frame, in_img,
		&anim_time_smootheststep, n_frames,
		"img/out/");

	rgbaimg_destroy(in_img);
	return 0;
}

int main_test_imprint(int argc, char *const argv[]) {
	rgba_image *out;
	double margin = 0.05;

	out = rgbaimg_create(500, 500);
	imprint_ext(out, &imprint_x2, &margin, (-1+0i), (+1+1i));
	png_save_to_file(out, "img/imprint.png");
	rgbaimg_destroy(out);

	return 0;
}


rgba_image *read_input_img(int argc, char *const argv[]) {
	char *imgid;
	char *path;
	rgba_image *input;

	if (getopt(argc, argv, "i:") != -1) {
		imgid = strdup(optarg);
	}

	sprintf_alloc(&path, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	png_load_from_file(&input, path);

	free(imgid);
	free(path);
	return input;
}

rgba_image *create_frame(double progress, const void *arg) {
	size_t w, h;
	const rgba_image *input = (const rgba_image *) arg;
	rgbaimg_get_dimensions(input, &w, &h);
	return warp_ext(
		input, &f, &progress,
		(-M_PI - M_PI*1i), (+M_PI + M_PI*1i),
		(-4-4i), (+4+4i),
		w, h);
}

void clean_dir(const char *path) {
	char *cmd;
	sprintf_alloc(&cmd, "rm -f %s/* --preserve-root", path);
	system(cmd);
	free(cmd);
}