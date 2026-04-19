#include <gtk/gtk.h>
// gcc base0.c -o base0 `pkg-config --cflags --libs gtk4`

static void on_activate(GApplication *app, gpointer user_data)
{
	GtkWidget *window;

#if	GTK_MAJOR_VERSION >= 4
	window = gtk_window_new();
#else
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_widget_show(window);
}

int main(int argc, char *argv[])
{
	GtkApplication *app;

	app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(G_OBJECT(app), "activate", G_CALLBACK(on_activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}