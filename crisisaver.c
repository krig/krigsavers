#include "util.h"

static gchar *geometry = NULL;
static int selectedtheme = 0;

static GOptionEntry options[] = {
	{"geometry",
	 0,
	 0,
	 G_OPTION_ARG_STRING,
	 &geometry,
	 "The initial size and position of window", "WIDTHxHEIGHT+X+Y"},
 	{"theme",
	 't',
	 0,
	 G_OPTION_ARG_INT,
	 &selectedtheme,
	 "Color theme", NULL},

       {NULL}
};

typedef struct ColorTheme_t
{
	const char* name;
	rgba_t bg;
	rgba_t fg0;
	rgba_t fg1;
	rgba_t fg2;
	rgba_t fg3;
} ColorTheme;

#define COLOR4i(r,g,b,a) { (double)(r)/255.0, (double)(g)/255.0, (double)(b)/255.0, (double)(a)/255.0 }

static ColorTheme g_themes[] = {
	{
		.name = "cinnamon",
		.bg = COLOR4i(32, 30, 24, 255),
		.fg0 = COLOR4i(246, 254, 255, 255),
		.fg1 = COLOR4i(245, 183, 65, 255),
		.fg2 = COLOR4i(68, 64, 57, 255),
		.fg3 = COLOR4i(42, 39, 32, 255),
	},
	{
		.name = "spice",
		.bg = COLOR4i(54, 41, 35, 255),
		.fg0 = COLOR4i(145, 130, 73, 255),
		.fg1 = COLOR4i(255, 245, 178, 255),
		.fg2 = COLOR4i(212, 192, 120, 255),
		.fg3 = COLOR4i(163, 34, 32, 255),
	},
	{
		.name = "sunshine",
		.bg = COLOR4i(112, 120, 100, 255),
		.fg0 = COLOR4i(193, 215, 78, 255),
		.fg1 = COLOR4i(245, 255, 124, 255),
		.fg2 = COLOR4i(223, 230, 180, 255),
		.fg3 = COLOR4i(166, 184, 156, 255),
	},
	{
		.name = "mexican",
		.bg = COLOR4i(36, 19, 35, 255),
		.fg0 = COLOR4i(43, 25, 49, 255),
		.fg1 = COLOR4i(82, 36, 55, 255),
		.fg2 = COLOR4i(191, 158, 165, 255),
		.fg3 = COLOR4i(248, 201, 207, 255),
	},
	{
		.name = "paragon",
		.bg = COLOR4i(37, 35, 38, 255),
		.fg0 = COLOR4i(242, 242, 242, 255),
		.fg1 = COLOR4i(242, 224, 189, 255),
		.fg2 = COLOR4i(242, 95, 41, 255),
		.fg3 = COLOR4i(28, 25, 38, 255),
	},
	{
		.name = "fauxberry",
		.bg = COLOR4i(64, 60, 61, 255),
		.fg0 = COLOR4i(166, 70, 78, 255),
		.fg1 = COLOR4i(140, 130, 116, 255),
		.fg2 = COLOR4i(242, 218, 189, 255),
		.fg3 = COLOR4i(89, 45, 53, 255),
	},
	{
		.name = "leaf",
		.bg = COLOR4i(50, 56, 54, 255),
		.fg0 = COLOR4i(186, 209, 181, 255),
		.fg1 = COLOR4i(219, 232, 207, 255),
		.fg2 = COLOR4i(240, 247, 232, 255),
		.fg3 = COLOR4i(255, 254, 245, 255),
	}


};

static ColorTheme* g_theme = g_themes + 1;

void set_source_rgb(cairo_t* cr, rgba_t* clr)
{
	cairo_set_source_rgb(cr, clr->r, clr->g, clr->b);
}

void set_source_rgba(cairo_t* cr, rgba_t* clr)
{
	cairo_set_source_rgba(cr, clr->r, clr->g, clr->b, clr->a);
}

void draw_text(cairo_t* cr, const char* text, double x, double y, double size, rgba_t* clr)
{
	cairo_save(cr);
	set_source_rgba(cr, clr);
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, size);
	cairo_move_to(cr, x, y);
	cairo_show_text(cr, text);
	//cairo_close_path(cr);
	cairo_restore(cr);
}

void draw_time(cairo_t* cr, time_t now, rgba_t* stroke, rgba_t* fill, double width, double height)
{
	double x = width - 200;
	double y = 100;
	double r = 80;
	double stroke_w = 10.0;
	cairo_save(cr);
	set_source_rgba(cr, fill);
	cairo_arc(cr, x, y, r, 0.0, 2 * M_PI);
	//cairo_stroke_preserve(cr);
	cairo_fill_preserve(cr);
	set_source_rgba(cr, stroke);
	cairo_set_line_width(cr, stroke_w);
	cairo_stroke(cr);
	cairo_restore(cr);
}

static gboolean
on_expose_event(GtkWidget      *widget,
		GdkEventExpose *event,
		gpointer        data)
{
	cairo_t *cr;
	time_t now;
	struct tm *now_tm;
	char cur_day[256];
	char cur_date[256];
	char cur_time[256];
	gint x, y;
	gint width, height;
	int invtim;
	cairo_text_extents_t extents;
	cairo_matrix_t mtx;

	now = time(0);
	now_tm = localtime(&now);

	strftime(cur_day, 256, "%A", now_tm);
	strftime(cur_date, 256, "%F", now_tm);
	strftime(cur_time, 256, "%T", now_tm);

	cr = gdk_cairo_create(widget->window);

	gdk_window_get_position(widget->window, &x, &y);
	gdk_drawable_get_size(widget->window, &width, &height);

	set_source_rgb (cr, &g_theme->bg);
	cairo_paint (cr);

	draw_text(cr, cur_day, 100, height-220, 80, &g_theme->fg0);
	draw_text(cr, cur_date, 100, height-160, 80, &g_theme->fg2);
	draw_text(cr, cur_time, 100, height-100, 80, &g_theme->fg1);

	//draw_time(cr, now, &g_theme->fg3, &g_theme->fg0, width, height);

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
			    "crisisaver",
			    options, NULL, &error);

	if (selectedtheme >= sizeof(g_themes)/sizeof(g_themes[0]))
		selectedtheme = 0;
	g_theme = g_themes + selectedtheme;

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

