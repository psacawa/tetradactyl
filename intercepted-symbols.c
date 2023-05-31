#include "tetradactyl.h"
#include <gtk/gtk.h>

extern GList *apps;

#define MACRO DECLARE_ORIG_LOC_PTR_EXTERN
#include "replaced-symbols"
#undef MACRO

void gtk_init() { orig_gtk_init(); }

GtkApplication *gtk_application_new(const gchar *application_id,
                                    GApplicationFlags flags) {
  tetradactyl_debug("in gtk_application_new %p \n", orig_gtk_application_new);
  GtkApplication *app = (*orig_gtk_application_new)(application_id, flags);
  /* TODO 30/05/20 psacawa: need to check if already created with same app-id?
   */
  apps = g_list_append(apps, app);
  g_signal_connect(app, "window-added",
                   G_CALLBACK(init_tetradactyl_key_capture), NULL);

  init_css();
  tetradactyl_debug("returning from gtk_application_new\n");
  return app;
}
