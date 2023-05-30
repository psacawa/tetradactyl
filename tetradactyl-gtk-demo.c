#include "gdk/gdkkeysyms.h"
#include "gtk/gtkcssprovider.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <glib.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TETRADACTYL_MODE_NORMAL = 0,
  TETRADACTYL_MODE_HINT,
} tetradactyl_mode = TETRADACTYL_MODE_NORMAL;

/* zmienna globalna */
GtkApplication *app;

#define NUM_BUTTONS 3

#define TETRADACTYL_HINT_KEY GDK_KEY_f
#define TETRADACTYL_FOLLOW_KEY GDK_KEY_f
#define TETRADACTYL_ESCAPE_KEY GDK_KEY_Escape

#define gtk_demo_debug(...) g_log("gtk-demo", G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define gtk_demo_info(...) g_log("gtk-demo", G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define gtk_demo_warning(...)                                                  \
  g_log("gtk-demo", G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define gtk_demo_error(...) g_log("gtk-demo", G_LOG_LEVEL_ERROR, __VA_ARGS__)

const char *hintchars = "ASDFGHJKLQWERTYUIOPZXCVBNM";
#define NUMHINTS strlen(hintchars)

int ishint_keyval(guint keyval) {
  /* TODO 27/05/20 psacawa: more robust */
  return (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) ||
         (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z);
}

typedef struct hintiter {
  unsigned int hintidx;
} hintiter;

void hint_overlay_for_active_window();
void clear_hints_for_active_window();
gboolean follow_hint(guint keyval, GdkKeyEvent *event);
void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget, hintiter *iter,
                      GArray *hint_widgets);

gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                        guint keycode, GdkModifierType state);
GtkOverlay *get_active_overlay(GtkApplication *app);

char *hint_next(hintiter *iter) {
  char *ret = malloc(2);
  if (iter->hintidx < strlen(hintchars)) {
    ret[0] = hintchars[iter->hintidx++];
    ret[1] = '\0';
  } else {
    /* dwuliterowe podpowiedzi */
  }
  return ret;
}

GtkOverlay *get_active_overlay(GtkApplication *app) {
  GtkWindow *window = gtk_application_get_active_window(app);
  GtkWidget *overlay = gtk_widget_get_first_child(GTK_WIDGET(window));
  return GTK_OVERLAY(overlay);
}

void button_press_cb(GtkWidget *receiver) {
  const char *label = gtk_button_get_label(GTK_BUTTON(receiver));
  printf("%s pressed\n", label);

  GtkWidget *parent = receiver;
  while (parent != NULL && !GTK_IS_WINDOW(parent)) {
    parent = gtk_widget_get_parent(parent);
  }
}

gboolean get_child_position_cb(GtkWidget *overlay, GtkWidget *hint,
                               GdkRectangle *allocation, gpointer user_data) {
  gtk_demo_info(
      "get_child_position_cb: widget:%s x=%d y=%d width=%d height=%d\n",
      gtk_widget_get_name(hint), allocation->x, allocation->y,
      allocation->width, allocation->height);

  GtkWidget *target = g_object_get_data(G_OBJECT(hint), "target");
  gtk_widget_measure(hint, GTK_ORIENTATION_HORIZONTAL, 0, NULL, NULL, NULL,
                     NULL); /* dummy - kills warnings */
  graphene_rect_t bounds;
  gboolean err;
  err = gtk_widget_compute_bounds(target, overlay, &bounds);
  if (err == FALSE) {
    g_error("gtk_widget_compute_bounds failed for %s",
            gtk_widget_get_name(target));

    return FALSE;
  }
  allocation->x = bounds.origin.x;
  allocation->y = bounds.origin.y;
  allocation->width = 20;
  allocation->height = 20;
  return TRUE;
}

GtkOverlay *init_tetradactyl(GtkWindow *win) {

  GtkWidget *child = gtk_window_get_child(win);
  if (GTK_IS_OVERLAY(child)) {
    return GTK_OVERLAY(child);
  }
  gtk_demo_debug("initializing tetradactyl");
  GtkWidget *overlay = gtk_overlay_new();
  gtk_window_set_child(win, overlay);
  gtk_overlay_set_child(GTK_OVERLAY(overlay), child);

  g_signal_connect(overlay, "get-child-position",
                   G_CALLBACK(get_child_position_cb), NULL);

  /* capture */
  GtkEventController *key_capture_controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(key_capture_controller,
                                             GTK_PHASE_CAPTURE);
  g_signal_connect_object(key_capture_controller, "key-pressed",
                          G_CALLBACK(key_pressed_cb), NULL, 0);
  gtk_event_controller_set_name(key_capture_controller,
                                "tetradactyl-controller");
  gtk_widget_add_controller(overlay, key_capture_controller);

  return GTK_OVERLAY(overlay);
}

int is_hintable(GtkWidget *widget) { return GTK_IS_BUTTON(widget); }

void hint_overlay_for_active_window() {
  GtkOverlay *overlay = get_active_overlay(app);

  /* spis elementów na overlay, skoro gtk nie udostępnia te dane?? */
  GArray *hint_widgets = g_object_get_data(G_OBJECT(overlay), "hint-widgets");
  if (hint_widgets != NULL) {
    g_array_free(hint_widgets, TRUE);
  }
  hint_widgets = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));

  GtkWidget *root_under_overlay =
      gtk_widget_get_first_child(GTK_WIDGET(overlay));
  hintiter iter = {0};
  hint_overlay_rec(overlay, root_under_overlay, &iter, hint_widgets);
  g_object_set_data(G_OBJECT(overlay), "hint-widgets", hint_widgets);
  tetradactyl_mode = TETRADACTYL_MODE_HINT;
}

void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget, hintiter *iter,
                      GArray *hint_widgets) {

  if (is_hintable(widget)) {
    char *hintstr = hint_next(iter);
    GtkWidget *hintlabel = gtk_label_new(hintstr);

    g_object_set_data(G_OBJECT(hintlabel), "target", widget);
    g_object_ref(widget);

    gtk_widget_add_css_class(hintlabel, "hintlabel");
    gtk_overlay_add_overlay(overlay, hintlabel);
    g_array_append_val(hint_widgets, hintlabel);
  }

  /* hintable does not preclude hintable children */
  GtkWidget *child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    hint_overlay_rec(overlay, child, iter, hint_widgets);
    child = gtk_widget_get_next_sibling(child);
  }
}

void clear_hints_for_active_window() {
  GtkOverlay *overlay = get_active_overlay(app);
  GArray *hint_widgets = g_object_get_data(G_OBJECT(overlay), "hint-widgets");
  for (int i = 0; i != hint_widgets->len; ++i) {
    GtkWidget *widget = g_array_index(hint_widgets, GtkWidget *, i);
    gtk_overlay_remove_overlay(overlay, widget);
  }
  tetradactyl_mode = TETRADACTYL_MODE_NORMAL;
}

void send_synthetic_click_event(GtkWidget *hint, GdkKeyEvent *orig_event) {
  GtkWindow *win = gtk_application_get_active_window(app);
  GtkButton *button = g_object_get_data(G_OBJECT(hint), "target");
  gtk_widget_activate(GTK_WIDGET(button));
}

/* zwróc czy stosowna podpowiedź została odnaleziona */
gboolean follow_hint(guint keyval, GdkKeyEvent *orig_event) {
  gtk_demo_info(__FUNCTION__);
  g_assert(tetradactyl_mode == TETRADACTYL_MODE_HINT);
  GtkOverlay *overlay = get_active_overlay(app);
  GArray *hint_widgets = g_object_get_data(G_OBJECT(overlay), "hint-widgets");
  for (int i = 0; i != hint_widgets->len; ++i) {
    GtkWidget *hint = g_array_index(hint_widgets, GtkWidget *, i);
    const char *hintlabel = gtk_label_get_text(GTK_LABEL(hint));
    if (hintlabel[0] == keyval) {
      gtk_demo_info("following hint with label %c", keyval);
      send_synthetic_click_event(hint, orig_event);
      clear_hints_for_active_window();
      return TRUE;
    }
  }
  return FALSE;
}

gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                        guint keycode, GdkModifierType state) {

  GdkKeyEvent *orig_event =
      (GdkKeyEvent *)(gtk_event_controller_get_current_event(controller));
  /* keycode odzwierciedla fizyczny klawisz i nie jest interesujący z
   * perspektywy naszej aplikacji */
  gtk_demo_info("key_pressed_cb: "
                "keyval=%#x=%d=%c "
                "state=%d",
                keyval, keyval, keyval, state);
  /* escape -> kill hints */
  /* tab -> iterate hints */
  /* alpha -> select hints */
  /* enter -> actvate hint */
  if (tetradactyl_mode == TETRADACTYL_MODE_NORMAL) {
    if (keyval == TETRADACTYL_HINT_KEY &&
        (state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_ALT_MASK)) == 0) {
      gtk_demo_debug("rendering hints");
      hint_overlay_for_active_window();
      return TRUE;
    }
  } else if (tetradactyl_mode == TETRADACTYL_MODE_HINT) {
    if (keyval == TETRADACTYL_ESCAPE_KEY) {
      clear_hints_for_active_window();
    } else if (ishint_keyval(keyval)) {
      follow_hint(toupper(keyval), orig_event);
    }
  }
  return FALSE;
}

void activate_cb(GtkApplication *app, gpointer *data) {
  gtk_demo_debug("app activate callback\n");

  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *buttons[NUM_BUTTONS];

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "my window");

  grid = gtk_grid_new();

  gtk_window_set_child(GTK_WINDOW(window), grid);

  /* buttons */
  char button_label[30];
  for (int i = 0; i != NUM_BUTTONS; ++i) {
    buttons[i] = gtk_button_new();
    sprintf(button_label, "button %d", i);
    gtk_button_set_label(GTK_BUTTON(buttons[i]), button_label);
    gtk_grid_attach(GTK_GRID(grid), buttons[i], i, 0, 1, 1);
    g_signal_connect(buttons[i], "clicked", G_CALLBACK(button_press_cb), NULL);
  }

  /* add css */
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, "./hints.css");
  GdkDisplay *display = gdk_display_get_default();
  gtk_style_context_add_provider_for_display(display,
                                             GTK_STYLE_PROVIDER(provider), 0);

  gtk_window_present(GTK_WINDOW(window));
  init_tetradactyl(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
  int status;

  app = gtk_application_new("org.gtk.demo", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate_cb), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
