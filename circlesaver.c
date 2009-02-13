#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "gs-theme-window.h"

static gchar *geometry = NULL;
static gboolean countdown = FALSE;

static GOptionEntry options[] = {
	{"geometry",
	 0,
	 0,
	 G_OPTION_ARG_STRING,
	 &geometry,
	 "The initial size and position of window", "WIDTHxHEIGHT+X+Y"},
	{"countdown",
	 'c',
	 0,
	 G_OPTION_ARG_NONE,
	 &countdown,
	 "If set, the clock counts down to 1234567890", NULL},

       {NULL}
};

#define NUM_CIRCLES 10
#define UPDATE_FREQ 10

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
	rgba_t rgba;
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

static double
next_random()
{
	return ((double)(rand()%1000)/500.0) - 1.0;
}

static circle_t
gen_circle(int i)
{
	int clr;
	circle_t c;
	c.x = 0.25 + sin(i*0.01) + next_random()*0.1;
	c.y = -0.5 + cos(i*0.001) + next_random()*0.1;
	c.r = 0.1 + next_random()*0.2;
	c.dx = 0.01 * sin(next_random());
	c.dy = -0.01 * cos(next_random());
	clr = rand()%2;
	switch (clr) {
	case 0: c.rgba = color(0x0fa080cf); break;
	case 1: c.rgba = color(0x0fb9b5fc); break;
	case 2: c.rgba = color(0x3f784f56); break;
	default:
	case 3: c.rgba = color(0x3fb1103c); break;
	};
	return c;
}

static void
update_circle(circle_t* c, double dt)
{
	c->x += c->dx * dt;
	c->y += c->dy * dt;
}

static void
draw_circle(cairo_t* cr, gint width, gint height, circle_t* c)
{
	cairo_set_source_rgba(cr, c->rgba.r, c->rgba.g, c->rgba.b, c->rgba.a);
	cairo_arc(cr, c->x*width, c->y*(double)height, c->r*(double)width, 0.0, 2 * M_PI);
	cairo_fill(cr);
	//cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.4);
	//cairo_set_line_width(cr, 8.0);
	//cairo_stroke(cr);
}

circle_t circles[NUM_CIRCLES];
cairo_pattern_t* bgpattern;

static gboolean
on_expose_event(GtkWidget      *widget,
		GdkEventExpose *event,
		gpointer        data)
{
	cairo_t *cr;
	gint x, y, width, height;
	int i;
	cr = gdk_cairo_create(widget->window);

	gdk_window_get_position(widget->window, &x, &y);
	gdk_drawable_get_size(widget->window, &width, &height);

	
	cairo_set_source_rgb (cr, 0, 0, 0);
	//cairo_set_source(cr, bgpattern);
	cairo_paint (cr);

	for (i = 0; i < NUM_CIRCLES; ++i) {
		draw_circle(cr, width, height, circles + i);
	}

	cairo_destroy(cr);

	return FALSE;
}

static gint
timeout_callback (gpointer data)
{
	double dt;
	int i;
	dt = (double)UPDATE_FREQ / 1000.0;
	for (i = 0; i < NUM_CIRCLES; ++i) {
		update_circle(circles + i, dt);
	}
	GtkWidget *darea = (GtkWidget*)data;
	gtk_widget_queue_draw (darea);
	return TRUE;
}

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *darea;
	GError *error;
	int i;
	rgba_t bgcolor1, bgcolor2;


	error = NULL;

	gtk_init_with_args (&argc, &argv,
			    "circlesaver",
			    options, NULL, &error);

	window = gs_theme_window_new();
	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER (window), darea);

	g_signal_connect(darea, "expose-event",
			 G_CALLBACK (on_expose_event), NULL);
	g_signal_connect(window, "destroy",
			 G_CALLBACK (gtk_main_quit), NULL);

	for (i = 0; i < NUM_CIRCLES; ++i) {
		circles[i] = gen_circle(i);
	}

	bgcolor1 = color(0xFF000000);
	bgcolor2 = color(0xFF705391);
	bgpattern = cairo_pattern_create_linear(100.5, 0.0, 100.5, 500.0);
	cairo_pattern_add_color_stop_rgb(bgpattern, 0.0, bgcolor1.r, bgcolor1.g, bgcolor1.b);
	cairo_pattern_add_color_stop_rgb(bgpattern, 1.0, bgcolor2.r, bgcolor2.g, bgcolor2.b);
	
	g_timeout_add_full (G_PRIORITY_HIGH, UPDATE_FREQ, timeout_callback, darea, NULL);
	//g_timeout_add (UPDATE_FREQ, timeout_callback, darea);

	if ((geometry == NULL)
	    || !gtk_window_parse_geometry (GTK_WINDOW (window), geometry))
		gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all(window);

	gtk_main();

	cairo_pattern_destroy(bgpattern);

	return 0;
}

