#include <gtk/gtk.h>
#include <stdio.h>

void greet_button_cb(GtkWidget *widget) { printf("hello\n"); }

void app_activate_cb(GtkApplication *app) {

  puts(__FUNCTION__);

  GtkWidget *win = gtk_application_window_new(app);
  GtkWidget *button = gtk_button_new();

  gtk_button_set_label(GTK_BUTTON(button), "greet");
  gtk_window_set_child(GTK_WINDOW(win), button);

  g_signal_connect(button, "clicked", G_CALLBACK(greet_button_cb), NULL);

  const char *accels[2] = {"F1", NULL};
  gtk_application_set_accels_for_action(app, "app.quit", accels);

  gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char *argv[]) {
  GtkApplication *app =
      gtk_application_new("org.gtk.GtkDemo", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), NULL);

  /* const char *accels[2] = {"<Control>q", NULL}; */

  g_application_run(G_APPLICATION(app), argc, argv);
  return 0;
}
