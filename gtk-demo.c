#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

void greet_button_cb(GtkWidget *widget, gpointer user_data) {
  printf("button%d\n", GPOINTER_TO_INT(user_data));
}

void app_activate_cb(GtkApplication *app) {
  GtkBuilder *builder = gtk_builder_new();
  gboolean ret = gtk_builder_add_from_file(builder, "demo.ui", NULL);
  if (ret == FALSE) {
    fprintf(stderr, "gtk_builder_add_from_file");
    exit(1);
  }
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
  gtk_application_add_window(app, GTK_WINDOW(window));
  gtk_window_present(GTK_WINDOW(window));

  for (int i = 0; i != 3; ++i) {
    char id[10];
    snprintf(id, sizeof(id) - 1, "button%d", i);
    GtkWidget *button = GTK_WIDGET(gtk_builder_get_object(builder, id));
    g_signal_connect(button, "clicked", G_CALLBACK(greet_button_cb),
                     GINT_TO_POINTER(i));
  }
}

int main(int argc, char *argv[]) {
  GtkApplication *app =
      gtk_application_new("org.gtk.GtkDemo", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);
  g_application_run(G_APPLICATION(app), argc, argv);
  return 0;
}
