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

#define MAX_SWARMERS 40
#define NUM_CIRCLES 4
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
	double stroke_w;
	rgba_t fill;
	rgba_t stroke;
	int i;
	int on;
} circle_t;

typedef struct swarmer_
{
    double x;
    double y;
    double r;
    double distance;
    double age;
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
	return g_rand_double_range(rander, 0.0, 2.0) - 1.0;
}

static double
next_random_01()
{
    if (rander == NULL)
        rander = g_rand_new_with_seed(time(0));
    return g_rand_double_range(rander, 0, 1.0);
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
	c.on = 0;
	c.i = i;
	c.stroke_w = 2.0;
	c.fill = color(0xffffffff);
	c.stroke = color(0xff1c222f);
	return c;
}

static void
gen_swarmer(circle_t* circle);

static double secounter = 0.0;

static void
update_circle(circle_t* c, double dt)
{
    int sec;
    secounter += dt/(double)NUM_CIRCLES;
    if ((int)secounter >= NUM_CIRCLES)
        secounter = 0.0;
    sec = (int)secounter;

    int n, i;
    //if (t % (c->i + 2)) {
    //if ((t&0x3) == ((i+1)&0x3)) {
    if (sec == c->i) {
        c->on = 1;
        c->fill = color(0xffffffff);
        c->stroke = color(0xff222222);
        
        n = (int)(next_random()*10.0) - 8;
        for (i = 0; i < n; ++i) {
            gen_swarmer(c);
        }
    } else {
        c->on = 0;
        c->fill = color(0xff000000);
        c->stroke = color(0xff000000);

        n = (int)(next_random()*50.0) - 48;
        for (i = 0; i < n; ++i) {
            gen_swarmer(c);
        }
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

circle_t circles[NUM_CIRCLES];
GPtrArray* swarmers = NULL;

static void
update_swarmer(swarmer_t* s, double dt);

double safesqrt(double st)
{
    if (st < 0.00001)
        return 0.00316;
    return sqrt(st);
}

static swarmer_t* swarmer_freelist = NULL;

static swarmer_t* 
alloc_swarmer()
{
    swarmer_t* ret = NULL;
    if (swarmer_freelist) {
        ret = swarmer_freelist;
        swarmer_freelist = *((swarmer_t**)swarmer_freelist);
    }
    else {
        ret = (swarmer_t*)malloc(sizeof(swarmer_t));
    }
    return ret;
}

static void
free_swarmer(swarmer_t* s)
{
    *((swarmer_t**)s) = swarmer_freelist;
    swarmer_freelist = s;
}

static void
gen_swarmer(circle_t* circle)
{
    if (swarmers->len >= MAX_SWARMERS)
        return;

    double dx, dy;
    double dist = CIRCLE_RADIUS + next_random_01()*0.05;
    swarmer_t* s = alloc_swarmer();
    s->distance = dist;
    s->circle = circle;
    s->x = circle->x;
    s->y = circle->y;
    s->age = 0.0;
    dx = next_random()*(CIRCLE_RADIUS+0.05);
    dy = safesqrt(dist*dist - dx*dx) * (next_random()>0.0 ? -1.0 : 1.0);

    s->x -= dx;
    s->y -= dy;
    
    update_swarmer(s, 0.0);
    g_ptr_array_add(swarmers, s);
}

static double
sqr_distance(double x1, double y1, double x2, double y2)
{
    return ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2));
}

static double
distance2D(double x1, double y1, double x2, double y2)
{
    double dist2 = sqr_distance(x1, y1, x2, y2);
    return safesqrt(dist2);
}

void scan_vicinity(swarmer_t* s,
    circle_t** evil,
    swarmer_t** friend,
    swarmer_t** enemy, 
    double* nearest_evil_circle, 
    double* nearest_friend, 
    double* nearest_enemy)
{
    double evil_dist = 1000.0;
    double friend_dist = 1000.0;
    double enemy_dist = 1000.0;
    int i;
    circle_t* circ = NULL;
    swarmer_t* s2 = NULL;
    
    *evil = NULL;
    *friend = NULL;
    *enemy = NULL;
    
    for (i = 0; i < NUM_CIRCLES; ++i) {
        circ = circles + i;
        if (circ == s->circle)
            continue;

        double dist = sqr_distance(s->x, circ->x, s->y, circ->y);
        if (evil_dist > dist) {
            evil_dist = dist;
            *evil = circ;
        }
    }
    
    int top = swarmers->len > 15 ? 15 : swarmers->len;
    for (i = 0; i < top; ++i) {
        s2 = g_ptr_array_index(swarmers, i);
        if (s2 == s)
            continue;
        double dist = sqr_distance(s->x, s2->x, s->y, s2->y);
        if (s2->circle == s->circle) {
            if (friend_dist > dist) {
                friend_dist = dist;
                *friend = s2;
            }
        }
        else {
            if (enemy_dist > dist) {
                enemy_dist = dist;
                *enemy = s2;
            }
        }
    }
    
    *nearest_evil_circle = safesqrt(evil_dist);
    *nearest_friend = safesqrt(friend_dist);
    *nearest_enemy = safesqrt(enemy_dist);
}

static void
update_swarmer(swarmer_t* s, double dt)
{
    circle_t* evil;
    swarmer_t* friend;
    swarmer_t* enemy;
    double nearest_evil;
    double nearest_friend;
    double nearest_enemy;
    
    if (dt) {
    scan_vicinity(s, &evil, &friend, &enemy, &nearest_evil, &nearest_friend, &nearest_enemy);
    
    // light attracts * 0.5
    // friend attracts * 0.25
    // evil repels * 0.7
    // enemy repels * 0.9
    
    double dx, dy;
    dx = next_random()*0.2;
    dy = next_random()*0.2;
    if (s->circle->on && s->distance < CIRCLE_RADIUS*1.3) {
        dx -= (s->circle->x - s->x) * 0.2;
        dy -= (s->circle->y - s->x) * 0.2;
    }
    else if (s->circle->on && s->distance < 0.3) {
        dx += (s->circle->x - s->x) * 0.66;
        dy += (s->circle->y - s->y) * 0.66;
    }
    if (nearest_friend < 0.5) {
        dx += (friend->x - s->x) * 0.25;
        dy += (friend->y - s->y) * 0.25;
    }
    if (evil->on && nearest_evil < 0.2) {
        dx -= (evil->x - s->x) * 0.7;
        dy -= (evil->y - s->y) * 0.7;
    }
        if (s->circle->on) {
            if (nearest_enemy < 0.1) {
                dx -= (enemy->x - s->x) * 0.05;
                dy -= (enemy->y - s->y) * 0.05;
            }   
        } else {
            if (nearest_enemy < 0.4) {
                dx -= (enemy->x - s->x) * 0.3;
                dy -= (enemy->y - s->y) * 0.3;
            }   
        }
    
    //dx += next_random()*s->age;
    //dy += next_random()*s->age;
    dx *= dt;
    dy *= dt;
    s->age += dt;
    s->x += dx;
    s->y += dy;
    }
    s->distance = distance2D(s->x, s->y, s->circle->x, s->circle->y);
}

static void
draw_swarmer(cairo_t* cr, gint width, gint height, swarmer_t* s)
{
    double intensity;
    rgba_t clr;
    switch (s->circle->i)
    {
        case 0: clr = color(0xFFFEE169); break;
        case 1: clr = color(0xFFCDD452); break;
        case 2: clr = color(0xFFF9722E); break;
        default:
        case 3: clr = color(0xFFC9313D); break;
    };
    double agelim = (s->age > 20.0 ? 20.0 : s->age) * 0.1; 
    intensity = 1.0 - (s->distance/0.25) + agelim*0.25;
	cairo_set_source_rgba(cr, clr.r*intensity, clr.g*intensity, clr.b*intensity, clr.a);
	cairo_rectangle(cr, s->x*width - 0.5 - agelim, s->y*(double)height - 0.5 - agelim, 1 + agelim*2.0, 1 + agelim*2.0);
	cairo_fill(cr);
}

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
    free_swarmer((swarmer_t*)swarmer);
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

    swarmers = g_ptr_array_sized_new(1000);

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

