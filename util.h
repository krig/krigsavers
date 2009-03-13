#ifdef UTIL_H__12234
#error Can only be included once
#endif

#ifndef UTIL_H__12234
#define UTIL_H__12234

#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <glib.h>
#include "gs-theme-window.h"

typedef struct rgba_
{
	double r, g, b, a;
} rgba_t;

typedef struct circle_
{
	double x;
	double y;
	double r;
	double dx;
	double dy;
	rgba_t fill;
	rgba_t stroke;
	double t;
} circle_t;

static rgba_t
color(unsigned int rgba)
{
	rgba_t clr;
	clr.a = ((double)((rgba>>24)&0xff))/255.5;
	clr.r = ((double)((rgba>>16)&0xff))/255.5;
	clr.g = ((double)((rgba>> 8)&0xff))/255.5;
	clr.b = ((double)((rgba    )&0xff))/255.5;
	return clr;
}

static GRand* gs_rander = NULL;

static void
free_rander(void)
{
	if (gs_rander)
		g_rand_free(gs_rander);
}

static void
init_random()
{
	if (gs_rander == NULL) {
		gs_rander = g_rand_new_with_seed(time(0));
		atexit(free_rander);
	}
}

static double
next_random()
{
	init_random();
	return g_rand_double_range(gs_rander, -1.0, 1.0);
}

static int
next_random_int_range(int begin, int end)
{
	init_random();
	return g_rand_int_range(gs_rander, begin, end);
}

#endif/*UTIL_H__12234*/
