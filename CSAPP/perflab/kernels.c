/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
/*
 * Please fill in the following team struct
 */
team_t team = {"Fuck nan yang bei yuan", /* Team name */

			   "Xin Tong",			   /* Second member full name (leave blank if none) */
			   "xin_tong@sjtu.edu.cn", /* Second member email addr (leave blank if none) */
			   "", ""};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/*
 * naive_rotate - The naive baseline version of rotate
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) {
	int i, j;

	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++) dst[RIDX(dim - 1 - j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr16[] = "rotate: 16x16 per block";
void rotate_16(int dim, pixel *src, pixel *dst) {
	// int k = 0;
	// for (int i = 0; i < dim; i++)
	// for (int j = 0; j < dim; j++) dst[RIDX(dim - 1 - j, i, dim)] = src[k++];

	int num = dim >> 4;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 4), jfrom = (j << 4);
			for (int a = 0; a < 16; a++)
				for (int b = 0; b < 16; b++) { dst[RIDX(dim - 1 - jfrom - b, ifrom + a, dim)] = src[RIDX(ifrom + a, jfrom + b, dim)]; }
		}
	}
}

char rotate_descr8[] = "rotate: 8x8 per block";
void rotate(int dim, pixel *src, pixel *dst) {
	// int k = 0;
	// for (int i = 0; i < dim; i++)
	// for (int j = 0; j < dim; j++) dst[RIDX(dim - 1 - j, i, dim)] = src[k++];

	int num = dim >> 3;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 3), jfrom = (j << 3);
			for (int a = 0; a < 8; a++)
				for (int b = 0; b < 8; b++) { dst[RIDX(dim - 1 - jfrom - b, ifrom + a, dim)] = src[RIDX(ifrom + a, jfrom + b, dim)]; }
		}
	}
}
/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_rotate_functions() {
	// add_rotate_function(&naive_rotate, naive_rotate_descr);
	// add_rotate_function(&rotate_16, rotate_descr16);
	add_rotate_function(&rotate, rotate_descr8);
	/* ... Register additional test functions here */
}

/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
	int red;
	int green;
	int blue;
	int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/*
 * initialize_pixel_sum - Initializes all fields of sum to 0
 */
static void initialize_pixel_sum(pixel_sum *sum) {
	sum->red = 0;
	sum->green = 0;
	sum->blue = 0;
	sum->num = 0;
	return;
}

/*
 * accumulate_sum - Accumulates field values of p in corresponding
 * fields of sum
 */
static void accumulate_sum(pixel_sum *sum, pixel p) {
	sum->red += (int)p.red;
	sum->green += (int)p.green;
	sum->blue += (int)p.blue;
	sum->num++;
	return;
}

/*
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) {
	current_pixel->red = (unsigned short)(sum.red / sum.num);
	current_pixel->green = (unsigned short)(sum.green / sum.num);
	current_pixel->blue = (unsigned short)(sum.blue / sum.num);
	return;
}

/*
 * avg - Returns averaged pixel value at (i,j)
 */

static pixel avg2(int dim, int i, int j, pixel *src) {
	int ii, jj;
	pixel_sum sum;
	pixel current_pixel;

	initialize_pixel_sum(&sum);
	int low1 = Max(i - 1, 0), up1 = Min(i + 1, dim - 1), low2 = Max(j - 1, 0), up2 = Min(j + 1, dim - 1);
	for (ii = low1; ii <= up1; ii++)
		for (jj = low2; jj <= up2; jj++) {
			sum.red += (int)(src[RIDX(ii, jj, dim)].red);
			sum.green += (int)(src[RIDX(ii, jj, dim)].green);
			sum.blue += (int)(src[RIDX(ii, jj, dim)].blue);
			sum.num++;
		}
	// accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);
	current_pixel.red = (unsigned short)(sum.red / sum.num);
	current_pixel.green = (unsigned short)(sum.green / sum.num);
	current_pixel.blue = (unsigned short)(sum.blue / sum.num);
	// assign_sum_to_pixel(&current_pixel, sum);
	return current_pixel;
}

static pixel avg(int dim, int i, int j, pixel *src) {
	int ii, jj;
	pixel_sum sum;
	pixel current_pixel;

	initialize_pixel_sum(&sum);
	for (ii = max(i - 1, 0); ii <= min(i + 1, dim - 1); ii++)
		for (jj = max(j - 1, 0); jj <= min(j + 1, dim - 1); jj++) accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

	assign_sum_to_pixel(&current_pixel, sum);
	return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) {
	int i, j;

	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++) dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth.
 * IMPORTANT: This is the version you will be graded on
 */
char smooth_descr[] = "smooth: smooth with blocking and no function";
void smooth_block_nofunc(int dim, pixel *src, pixel *dst) {
	int num = dim >> 3;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 3), jfrom = (j << 3);
			for (int a = 0; a < 8; a++)
				for (int b = 0; b < 8; b++) { dst[RIDX(ifrom + a, jfrom + b, dim)] = avg2(dim, ifrom + a, jfrom + b, src); }
		}
	}
	// for (i = 0; i < dim; i++)
	// 	for (j = 0; j < dim; j++);
}

char smooth_descr[] = "smooth: Current working version";
void smooth_block_nofunc(int dim, pixel *src, pixel *dst) {
	int num = dim >> 3;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 3), jfrom = (j << 3);
			for (int a = 0; a < 8; a++)
				for (int b = 0; b < 8; b++) { dst[RIDX(ifrom + a, jfrom + b, dim)] = avg2(dim, ifrom + a, jfrom + b, src); }
		}
	}
	// for (i = 0; i < dim; i++)
	// 	for (j = 0; j < dim; j++);
}
/*********************************************************************
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.
 *********************************************************************/

void register_smooth_functions() {
	add_smooth_function(&smooth_block_nofunc, smooth_descr);
	add_smooth_function(&naive_smooth, naive_smooth_descr);
	/* ... Register additional test functions here */
}
