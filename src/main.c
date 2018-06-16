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
#include "util/imprint.h"

#define IMG_PATH_PREFIX "img/"
#define IMG_PATH_SUFFIX ".png"

#define FRAMES_OUTPUT_DIR "img/out/"
#define OUTPUT_FPS 60


double complex f(double complex z, const void *arg) {
	double t = *((const double *) arg);
	return cpow(z, clerp(1, 3, t));
}

void clean_dir(const char *path);

void read_input(int argc, char *const argv[], rgba_image **img, size_t *n_frames);
rgba_image *create_frame(double progress, const void *arg);

int create_imprint(int argc, char *const argv[]);
int create_anim(int argc, char *const argv[]);

int main(int argc, char *const argv[]) {
	int c = getopt(argc, argv, "sa");
	if (c == 's') {
		return create_imprint(argc, argv);
	} else {
		return create_anim(argc, argv);
	}
}

int create_anim(int argc, char *const argv[]) {
	rgba_image *in_img;
	size_t n_frames;
	size_t inwidth, inheight;

	read_input(argc, argv, &in_img, &n_frames);
	rgbaimg_get_dimensions(in_img, &inwidth, &inheight);
	omp_set_nested(true);
	
	clean_dir(FRAMES_OUTPUT_DIR);

	animate(
		&create_frame, in_img,
		&anim_time_smootheststep, n_frames,
		"img/out/");

	rgbaimg_destroy(in_img);
	return 0;
}

int create_imprint(int argc, char *const argv[]) {
	rgba_image *out;
	rgba_pixel white = { .r = 255, .g = 255, .b = 255, .a = 127 };
	rgba_pixel black = { .r = 0, .g = 0, .b = 0, .a = 255 };

	out = rgbaimg_create(500, 500);
	imprint_rect(
		out, (-1-1i), (+1+1i),
		black,
		(-0.5-0.5i), (+0.5+0.5i));
	imprint_circle(
		out, (-1-1i), (+1+1i),
		white,
		(0.5+0i),
		0.0, 1.0,
		M_PI_2, M_PI);
	imprint_line(
		out, (-1-1i), (+1+1i),
		white, 0.1,
		1, -0.5, 0);
	imprint_line(
		out, (-1-1i), (+1+1i),
		white, 0.1,
		1, 1, 0);
	imprint_line(
		out, (-1-1i), (+1+1i),
		white, 0.1,
		1, 0, 0);
	png_save_to_file(out, "img/imprint.png");
	rgbaimg_destroy(out);

	return 0;
}


void read_input(int argc, char *const argv[], rgba_image **img, size_t *n_frames) {
	char *imgid;
	char *path;
	int c;
	float out_duration;

	while ((c = getopt(argc, argv, "i:t:")) != -1) {
		switch (c) {
			case 't':
				sscanf(optarg, "%f", &out_duration);
				*n_frames = out_duration * OUTPUT_FPS;
				break;
			case 'i':
				imgid = strdup(optarg);
				break;
		}
	}

	sprintf_alloc(&path, "%s%s%s", IMG_PATH_PREFIX, imgid, IMG_PATH_SUFFIX);
	png_load_from_file(img, path);

	free(imgid);
	free(path);
}

rgba_image *create_frame(double progress, const void *arg) {
	size_t w, h;
	const rgba_image *input = (const rgba_image *) arg;
	rgbaimg_get_dimensions(input, &w, &h);
	return warp_ext(
		input, &f, &progress,
		(-1-1i), (+1+1i),
		(-1-1i), (+1+1i),
		w, h);
}

void clean_dir(const char *path) {
	char *cmd;
	sprintf_alloc(&cmd, "rm -f %s/* --preserve-root", path);
	system(cmd);
	free(cmd);
}