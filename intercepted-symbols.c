#define G_LOG_USE_STRUCTURED
#include "tetradactyl.h"
#include <gtk/gtk.h>

void PUBLIC gtk_init() { orig_gtk_init(); }

GtkApplication PUBLIC *gtk_application_new(const gchar *application_id,
                                           GApplicationFlags flags) {
  tetradactyl_debug("in gtk_application_new %p \n", orig_gtk_application_new);
  GtkApplication *app = (*orig_gtk_application_new)(application_id, flags);

  if (gtk_app) {
    tetradactyl_critical("creating additional GtkApplication. Only one per "
                         "program is supported");
  }
  gtk_app = app;

  /* update_tetradactyl_key_capture(gtk_app); */
  g_signal_connect(app, "window-added",
                   G_CALLBACK(update_tetradactyl_key_capture), NULL);

  return app;
}

GtkWidget PUBLIC *gtk_window_new() {
  GtkWidget *ret = (*orig_gtk_window_new)();
  return ret;
}
