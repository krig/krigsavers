#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gs-theme-window.h"

static gchar *geometry = NULL;
static gboolean countdown = FALSE;
static gboolean g_variant1 = FALSE;
static gboolean g_colour_blue = FALSE;

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

	{"variant1",
	 0,
	 0,
	 G_OPTION_ARG_NONE,
	 &g_variant1,
	 "clock variant 1", NULL},

	{"blue",
	 0,
	 0,
	 G_OPTION_ARG_NONE,
	 &g_colour_blue,
	 "blue colour scheme", NULL},
	
       {NULL}
};


static gboolean
on_expose_event(GtkWidget      *widget,
		GdkEventExpose *event,
		gpointer        data)
{
	cairo_t *cr;
	time_t now;
	struct tm *now_tm;
	char snow[256];
	char smax[256];
	char* ptr;
	gint x, y;
	gint width, height;
	int invtim;
	cairo_text_extents_t extents;
	cairo_matrix_t mtx;

	now = time(0);
	now_tm = localtime(&now);

	if (countdown) {
		strftime(snow, 256, "%s", now_tm);
		invtim = 1234567890 - atoi(snow);
		sprintf(snow, "%d", invtim);
	}
	else if (g_variant1) {
		strftime(snow, 256, "%T", now_tm);
	}
	else {
		strftime(snow, 256, "%s", now_tm);
	}

	strcpy(smax, snow);
	for (ptr = smax; *ptr; ++ptr) {
		*ptr = '9';
	}

	cr = gdk_cairo_create(widget->window);

	gdk_window_get_position(widget->window, &x, &y);
	gdk_drawable_get_size(widget->window, &width, &height);

	double bg[3] = {0.0, 0.0, 0.0};
	double fg[4] = {1.0, 0.4, 0.0, 1.0};
	double fg_shadow[4] = {1.0, 0.4, 0.0, 0.2};

	if (g_colour_blue) {
		fg[0] = 0.0;
		fg[2] = 1.0;
		fg_shadow[0] = 0.0;
		fg_shadow[2] = 1.0;
	}

	cairo_set_source_rgb (cr, bg[0], bg[1], bg[2]);
	cairo_paint (cr);

	cairo_set_source_rgba(cr, fg[0], fg[1], fg[2], fg[3]);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size(cr, 80.0);


	cairo_text_extents(cr,
			   smax,
			   &extents);

	cairo_move_to(cr, (width - extents.width)/2.0, (height - extents.height)/2.0);


	cairo_show_text(cr, snow);

	cairo_set_source_rgba(cr, fg_shadow[0], fg_shadow[1], fg_shadow[2], fg_shadow[3]);
	cairo_move_to(cr, (width - extents.width)/2.0, (height - extents.height)/2.0);
	cairo_matrix_init_scale(&mtx, 80.0, -80.0);
	cairo_set_font_matrix(cr, &mtx);

	cairo_show_text(cr, snow);

	cairo_destroy(cr);

	return FALSE;
}

static gint
timeout_callback (gpointer data)
{
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

	error = NULL;

	gtk_init_with_args (&argc, &argv,
			    "epochsaver",
			    options, NULL, &error);

	window = gs_theme_window_new();
	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER (window), darea);

	g_signal_connect(darea, "expose-event",
			 G_CALLBACK (on_expose_event), NULL);
	g_signal_connect(window, "destroy",
			 G_CALLBACK (gtk_main_quit), NULL);

	g_timeout_add (1000, timeout_callback, darea);

	if ((geometry == NULL)
	    || !gtk_window_parse_geometry (GTK_WINDOW (window), geometry))
		gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}

