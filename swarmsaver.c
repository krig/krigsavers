#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <glib.h>
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

#define NUM_CIRCLES 4
#define UPDATE_FREQ 20

typedef struct rgba_
{
	double r, g, b, a;
} rgba_t;

typedef struct circle_
{
	double x;
	double y;
	double r;
	double stroke_w;
	rgba_t fill;
	rgba_t stroke;
	int i;
} circle_t;

typedef struct swarmer_
{
    double x;
    double y;
    double r;
    double distance;
    circle_t* circle;
} swarmer_t;

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

GRand* rander = NULL;
static double
next_random()
{
	if (rander == NULL)
		rander = g_rand_new_with_seed(time(0));
	return g_rand_double_range(rander, -1.0, 1.0);
}

#define CIRCLE_RADIUS 0.015
#define CIRCLE_SIDESPACE 0.3
#define CIRCLE_SPACING ((1.0 - ((CIRCLE_SIDESPACE + CIRCLE_RADIUS)*2.0))/(NUM_CIRCLES-1.0))

static void
calc_circle_pos(int i, double* x, double* y)
{
    *x = CIRCLE_SIDESPACE + CIRCLE_RADIUS + (double)i * CIRCLE_SPACING;
    *y = 0.5;
}

static circle_t
gen_circle(int i)
{
	circle_t c;
	calc_circle_pos(i, &c.x, &c.y);
	c.r = CIRCLE_RADIUS;
	c.i = i;
	c.stroke_w = 2.0;
	c.fill = color(0xffffffff);
	c.stroke = color(0xff1c222f);
	return c;
}

static void
gen_swarmer(circle_t* circle);

static void
update_circle(circle_t* c, double dt)
{
    time_t t;
    int n, i;
    t = time(0);
    if (t % (c->i + 2)) {
        c->fill = color(0xffffffff);
        
        n = (int)(next_random()*10.0) - 5;
        for (i = 0; i < n; ++i) {
            gen_swarmer(c);
        }
    } else {
        c->fill = color(0xff000000);
    }
}

static void
draw_circle(cairo_t* cr, gint width, gint height, circle_t* c)
{
	cairo_set_source_rgba(cr, c->fill.r, c->fill.g, c->fill.b, c->fill.a);
	cairo_arc(cr, c->x*width, c->y*(double)height, c->r*(double)width, 0.0, 2 * M_PI);
	//cairo_stroke_preserve(cr);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr, c->stroke.r, c->stroke.g, c->stroke.b, c->stroke.a);
	cairo_set_line_width(cr, c->stroke_w);
	cairo_stroke(cr);
}

GPtrArray* swarmers = NULL;
static void
update_swarmer(swarmer_t* s, double dt);

static void
gen_swarmer(circle_t* circle)
{
    double dx, dy;
    double dist = 0.05 + next_random()*0.02;
    swarmer_t* s = malloc(sizeof(swarmer_t));
    s->distance = dist;
    s->circle = circle;
    s->x = circle->x;
    s->y = circle->y;
    dx = next_random()*0.07;
    dy = sqrt(dist*dist - dx*dx) * (next_random()>0.0 ? -1.0 : 1.0);
    
    s->x += dx;
    s->y += dy;
    
    update_swarmer(s, 0.0);
    g_ptr_array_add(swarmers, s);
}

#define DISTANCE(x1,y1,x2,y2) sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2))

static void
update_swarmer(swarmer_t* s, double dt)
{
    double dx, dy;
    dx = -(s->circle->x - s->x);
    dy = -(s->circle->y - s->y);
    dx += next_random()*0.05;
    dy += next_random()*0.05;
    dx *= dt;
    dy *= dt;
    s->x += dx;
    s->y += dy;
    s->distance = DISTANCE(s->x, s->circle->x, s->y, s->circle->y);
    s->r = 0.005;//0.005 + DISTANCE(s->x, s->circle->x, s->y, s->circle->y) * 0.01;
    if (s->r > 0.04)
        s->r = 0.04;
}

static void
draw_swarmer(cairo_t* cr, gint width, gint height, swarmer_t* s)
{
    double intensity;
    rgba_t clr;
    switch (s->circle->i)
    {
        case 0: clr = color(0xFFFFF0F0); break;
        case 1: clr = color(0xFFF0FFF0); break;
        case 2: clr = color(0xFFF0F0FF); break;
        case 3: clr = color(0xFFFFFFFF); break;
    };
    intensity = 4.0*(0.25 - s->distance); 
	cairo_set_source_rgba(cr, clr.r*intensity, clr.g*intensity, clr.b*intensity, clr.a);
	//cairo_arc(cr, s->x*width, s->y*(double)height, s->r*(double)width, 0.0, 2 * M_PI);
	cairo_rectangle(cr, s->x*width - 0.5, s->y*(double)height - 0.5, 1, 1);
	cairo_fill(cr);
}

circle_t circles[NUM_CIRCLES];

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
	cairo_paint (cr);

	for (i = 0; i < NUM_CIRCLES; ++i) {
		draw_circle(cr, width, height, circles + i);
	}
	
	for (i = 0; i < swarmers->len; ++i) {
	    if (g_ptr_array_index(swarmers, i)) {
	        draw_swarmer(cr, width, height, (swarmer_t*)g_ptr_array_index(swarmers, i));
	    }
	}

	cairo_destroy(cr);

	return FALSE;
}

static void
update_swarmer_cb(gpointer swarmer, gpointer dt)
{
    update_swarmer((swarmer_t*)swarmer, *(double*)dt);
}

static gint
timeout_callback (gpointer data)
{
    swarmer_t* swarmer;
	double dt;
	int i;
	dt = (double)UPDATE_FREQ / 1000.0;
	for (i = 0; i < NUM_CIRCLES; ++i) {
		update_circle(circles + i, dt);
	}
	g_ptr_array_foreach(swarmers, update_swarmer_cb, &dt);
	for (i = 0; i < swarmers->len; ++i) {
	    swarmer = (swarmer_t*)g_ptr_array_index(swarmers, i);
	    if (swarmer->distance > 0.25) {
	        free(swarmer);
	        g_ptr_array_remove_index_fast(swarmers, i);
	        --i;
	    }
	}
	GtkWidget *darea = (GtkWidget*)data;
	gtk_widget_queue_draw (darea);
	return TRUE;
}

void free_swarmer_cb(gpointer swarmer, gpointer user_data)
{
    free(swarmer);
}

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *darea;
	GError *error;
	int i;


	error = NULL;

	gtk_init_with_args (&argc, &argv,
			    "circlesaver",
			    options, NULL, &error);

    swarmers = g_ptr_array_sized_new(100);

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
	
	g_timeout_add_full (G_PRIORITY_HIGH, UPDATE_FREQ, timeout_callback, darea, NULL);
	//g_timeout_add (UPDATE_FREQ, timeout_callback, darea);

	if ((geometry == NULL)
	    || !gtk_window_parse_geometry (GTK_WINDOW (window), geometry))
		gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all(window);

	gtk_main();

    g_ptr_array_foreach(swarmers, &free_swarmer_cb, NULL);
    g_ptr_array_free(swarmers, TRUE);
    
	if (rander)
		g_rand_free(rander);

	return 0;
}

