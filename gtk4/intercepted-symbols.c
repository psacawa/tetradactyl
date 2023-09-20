/* Copyright 2023 Paweł Sacawa. All rights reserved. */
#include <stdarg.h>
#define G_LOG_USE_STRUCTURED
#include "init.h"
#include "tetradactyl.h"
#include <glib-object.h>
#include <gtk/gtk.h>

void handle_gtk_application_added(GtkApplication *app);

/* SYMBOLS */

PUBLIC void gtk_init() { orig_gtk_init(); }

PUBLIC GtkApplication *gtk_application_new(const gchar *application_id,
                                           GApplicationFlags flags) {
  tetradactyl_debug("in gtk_application_new %p \n", orig_gtk_application_new);
  GtkApplication *app = (*orig_gtk_application_new)(application_id, flags);
  handle_gtk_application_added(app);
  return app;
}

GtkWidget PUBLIC *gtk_window_new() {
  GtkWidget *ret = (*orig_gtk_window_new)();
  return ret;
}

/* This is lifted from glib/gobject/gobject.c: a license violation */
/* The issue with regular proxying is proxying variadic args and passing them
 * on. This implies manually implementing in assembly the system's underlying
 * ABI for argument passing, This includes e.g. floats. Either that or try to
 * erase the current stack frame entirely... or perform the variadic call with
 * libffi, but it doesn't work right now */
PUBLIC gpointer g_object_new(GType object_type,
                             const gchar *first_property_name, ...) {
  GObject *object;
  va_list var_args;

  /* short circuit for calls supplying no properties */
  if (!first_property_name)
    return g_object_new_with_properties(object_type, 0, NULL, NULL);

  va_start(var_args, first_property_name);
  object = g_object_new_valist(object_type, first_property_name, var_args);
  va_end(var_args);

  /* MODIFIED */
  if (g_type_is_a(object_type, GTK_TYPE_WINDOW)) {
    /* handle window init */
  } else if (g_type_is_a(object_type, G_TYPE_APPLICATION)) {
    handle_gtk_application_added(GTK_APPLICATION(object));
  }
  /* MODIFIED */

  return object;
}

/* AUXILIARY */

void handle_gtk_application_added(GtkApplication *app) {
  if (gtk_app) {
    tetradactyl_critical("creating additional GtkApplication. Only one per "
                         "program is supported");
  }
  gtk_app = app;

  /* update_tetradactyl_key_capture(gtk_app); */
  g_signal_connect(app, "window-added",
                   G_CALLBACK(update_tetradactyl_key_capture), NULL);
}
