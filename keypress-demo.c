
/* event-controller.c
 *
 * Compile: cc -ggdb event-controller.c -o event-controller $(pkg-config
 * --cflags --libs gtk4) -o event-controller Run: ./event-controller
 *
 * Author: Mohammed Sadiq <www.sadiqpk.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later OR CC0-1.0
 */

#include <gtk/gtk.h>

#define REPEAT_MS 100

int x_offset, y_offset;
guint key_repeat_id;
guint key_val;

gboolean update_square(gpointer user_data) {
  GtkWidget *drawing_area = user_data;

  g_assert(GTK_IS_DRAWING_AREA(drawing_area));

  if (key_val == GDK_KEY_Left)
    x_offset--;
  else if (key_val == GDK_KEY_Right)
    x_offset++;
  else if (key_val == GDK_KEY_Up)
    y_offset--;
  else if (key_val == GDK_KEY_Down)
    y_offset++;

  /* Request to redraw */
  gtk_widget_queue_draw(drawing_area);

  return G_SOURCE_CONTINUE;
}

static gboolean event_key_pressed_cb(GtkWidget *drawing_area, guint keyval,
                                     guint keycode, GdkModifierType state,
                                     GtkEventControllerKey *event_controller) {
  g_assert(GTK_IS_DRAWING_AREA(drawing_area));

  if (state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_ALT_MASK))
    return FALSE;

  key_val = keyval;

  g_clear_handle_id(&key_repeat_id, g_source_remove);
  key_repeat_id = g_timeout_add(REPEAT_MS, update_square, drawing_area);
  update_square(drawing_area);

  return TRUE;
}

static gboolean event_key_released_cb(GtkWidget *drawing_area) {
  /* Stop moving the square regardless of which key was released */
  g_clear_handle_id(&key_repeat_id, g_source_remove);

  return FALSE;
}

static void draw_func(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                      int height, gpointer user_data) {
  int x, y;

  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); /* white */
  cairo_paint(cr);

  x = CLAMP(10 + x_offset, 0, width - 90);
  y = CLAMP(10 + y_offset, 0, height - 90);

  cairo_set_source_rgb(cr, 0.2, 0.3, 0.8);
  cairo_rectangle(cr, x, y, 90, 90);
  cairo_fill(cr);
  cairo_save(cr);
}

static void app_activated_cb(GtkApplication *app) {
  GtkEventController *event_controller;
  GtkWindow *window;
  GtkWidget *child;

  window = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_default_size(window, 360, 540);

  child = gtk_drawing_area_new();
  gtk_window_set_child(window, child);
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(child), draw_func, app, NULL);

  event_controller = gtk_event_controller_key_new();

  g_signal_connect_object(event_controller, "key-pressed",
                          G_CALLBACK(event_key_pressed_cb), child,
                          G_CONNECT_SWAPPED);
  g_signal_connect_object(event_controller, "key-released",
                          G_CALLBACK(event_key_released_cb), child,
                          G_CONNECT_SWAPPED);
  gtk_widget_add_controller(GTK_WIDGET(window), event_controller);

  gtk_window_present(window);
}

int main(int argc, char *argv[]) {
  g_autoptr(GtkApplication) app = gtk_application_new(NULL, 0);

  g_signal_connect(app, "activate", G_CALLBACK(app_activated_cb), NULL);

  return g_application_run(G_APPLICATION(app), argc, argv);
}
