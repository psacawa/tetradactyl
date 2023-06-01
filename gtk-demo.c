#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_BUTTONS 6

void greet_button_cb(GtkWidget *widget, gpointer user_data) {
  printf("button%d\n", GPOINTER_TO_INT(user_data));
}

void app_activate_cb(GtkApplication *app) {
  GtkBuilder *builder = gtk_builder_new();
  gboolean ret = gtk_builder_add_from_file(builder, "gtk-demo.ui", NULL);
  if (ret == FALSE) {
    fprintf(stderr, "gtk_builder_add_from_file");
    exit(1);
  }
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  gtk_application_add_window(app, GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(window));

  for (int i = 0; i != NUM_BUTTONS; ++i) {
    char id[10];
    snprintf(id, sizeof(id) - 1, "button%d", i);
    GtkWidget *button = GTK_WIDGET(gtk_builder_get_object(builder, id));
    g_signal_connect(button, "activate", G_CALLBACK(greet_button_cb),
                     GINT_TO_POINTER(i));
  }
}

/* randomly generate id each time, to avoid dbus name collision */
static const char *get_appliction_id() {
  char *buf = malloc(20);
  GRand *rng = g_rand_new();
  sprintf(buf, "org.gtk.GtkDemo%d", g_rand_int(rng));
  return buf;
}

int main(int argc, char *argv[]) {
  GtkApplication *app =
      gtk_application_new(get_appliction_id(), G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
  g_application_run(G_APPLICATION(app), argc, argv);
  return 0;
}
