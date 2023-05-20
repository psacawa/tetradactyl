#include "gdk/gdkkeysyms.h"
#include "gtk/gtkcssprovider.h"
#include <glib.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int print_trees = 0;
enum {
  TETRADACTYL_MODE_NORMAL = 0,
  TETRADACTYL_MODE_HINT,
} tetradactyl_mode = TETRADACTYL_MODE_NORMAL;

/* zmienna globalna */
GtkApplication *app;

#define NUM_BUTTONS 3
#define print_tree(w) print_tree_rec(w, 0)

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

typedef struct hintiter {
  unsigned int hintidx;
} hintiter;

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

void print_tree_rec(GtkWidget *widget, int rec);
void apply_css_rec(GtkWidget *rec, GtkCssProvider *provider);
void hint_overlay(GtkOverlay *overlay);
void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget, hintiter *iter);

void add_provider_to_widget(GtkWidget *widget, GtkCssProvider *provider) {
  GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
  gtk_style_context_add_provider(ctx, GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void button_press_cb(GtkWidget *receiver) {
  const char *label = gtk_button_get_label(GTK_BUTTON(receiver));
  printf("%s pressed\n", label);

  GtkWidget *parent = receiver;
  while (parent != NULL && !GTK_IS_WINDOW(parent)) {
    parent = gtk_widget_get_parent(parent);
  }

  if (print_trees) {
    print_tree(parent);
  }
}

void render_hint(GtkOverlay *root, char *hintchar, GtkAllocation *allocation) {}

gboolean get_child_position_cb(GtkWidget *overlay, GtkWidget *hint,
                               GdkRectangle *allocation, gpointer user_data) {
  printf("get_child_position_cb: widget:%s x=%d y=%d width=%d height=%d\n",
         gtk_widget_get_name(hint), allocation->x, allocation->y,
         allocation->width, allocation->height);

  GtkWidget *target = g_object_get_data(G_OBJECT(hint), "target");
  graphene_rect_t bounds;
  gboolean err;
  err = gtk_widget_compute_bounds(target, overlay, &bounds);
  if (err == FALSE) {
    gtk_demo_error("gtk_widget_compute_bounds failed for %s",
                   gtk_widget_get_name(target));

    return FALSE;
  }
  allocation->x = bounds.origin.x;
  allocation->y = bounds.origin.y;
  allocation->width = 20;
  allocation->height = 20;
  return TRUE;
}

GtkOverlay *init_overlay(GtkWindow *win) {
  GtkWidget *child = gtk_window_get_child(win);
  if (GTK_IS_OVERLAY(child)) {
    return GTK_OVERLAY(child);
  }
  gtk_demo_debug("injecting overlay");
  GtkWidget *overlay = gtk_overlay_new();
  gtk_window_set_child(win, overlay);
  gtk_overlay_set_child(GTK_OVERLAY(overlay), child);

  g_signal_connect(overlay, "get-child-position",
                   G_CALLBACK(get_child_position_cb), NULL);

  /* apply css */
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, "./hints.css");
  g_object_set_data(G_OBJECT(overlay), "css-provider", (gpointer)provider);
  /* apply_css_rec(window, provider); */

  return GTK_OVERLAY(overlay);
}

int is_hintable(GtkWidget *widget) { return GTK_IS_BUTTON(widget); }

void hint_overlay(GtkOverlay *overlay) {
  GtkWidget *root_under_overlay =
      gtk_widget_get_first_child(GTK_WIDGET(overlay));
  hintiter iter = {0};
  hint_overlay_rec(overlay, root_under_overlay, &iter);
  tetradactyl_mode = TETRADACTYL_MODE_HINT;
}

void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget, hintiter *iter) {

  if (is_hintable(widget)) {
    GtkCssProvider *provider =
        g_object_get_data(G_OBJECT(overlay), "css-provider");
    add_provider_to_widget(GTK_WIDGET(widget), provider);
    char *hintstr = hint_next(iter);
    GtkWidget *hintlabel = gtk_label_new(hintstr);

    g_object_set_data(G_OBJECT(hintlabel), "target", widget);
    g_object_ref(widget);

    add_provider_to_widget(GTK_WIDGET(hintlabel), provider);
    gtk_widget_add_css_class(hintlabel, "hintlabel");
    gtk_overlay_add_overlay(overlay, hintlabel);
  }

  /* hintable does not preclude hintable children */
  GtkWidget *child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    hint_overlay_rec(overlay, child, iter);
    child = gtk_widget_get_next_sibling(child);
  }
}

gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                        guint keycode, GdkModifierType state) {
  gtk_demo_info("key_pressed_cb: "
                "  keyval:  %#x=%d=%c "
                "  keycode: %5d "
                "  state:   %5d",
                keyval, keyval, keyval, keycode, state);
  if (tetradactyl_mode == TETRADACTYL_MODE_NORMAL) {
    if (keyval == TETRADACTYL_HINT_KEY && state == 0) {
      gtk_demo_debug("rendering hints");
      GtkWindow *window = gtk_application_get_active_window(app);
      GtkWidget *overlay = gtk_widget_get_first_child(GTK_WIDGET(window));
      g_assert(GTK_IS_OVERLAY(overlay));
      hint_overlay((GTK_OVERLAY(overlay)));
      return TRUE;
    }
  } else if (tetradactyl_mode == TETRADACTYL_MODE_HINT) {
    /* escape -> kill hints */
    /* tab -> iterate hints */
    /* alpha -> select hints */
    /* enter -> actvate hint */
  }
  return FALSE;
}

void activate_cb(GtkApplication *app, gpointer *data) {
  gtk_demo_debug("app activate callback\n");

  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *buttons[NUM_BUTTONS];

  GtkEventController *key_capture_controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(key_capture_controller,
                                             GTK_PHASE_CAPTURE);
  g_signal_connect_object(key_capture_controller, "key-pressed",
                          G_CALLBACK(key_pressed_cb), NULL, 0);

  window = gtk_application_window_new(app);
  gtk_widget_add_controller(window, key_capture_controller);
  gtk_window_set_title(GTK_WINDOW(window), "my window");

  grid = gtk_grid_new();
  gtk_window_set_child(GTK_WINDOW(window), grid);

  /* buttons */
  char buttton_label[30];
  for (int i = 0; i != NUM_BUTTONS; ++i) {
    buttons[i] = gtk_button_new();
    sprintf(buttton_label, "button %d", i);
    gtk_button_set_label(GTK_BUTTON(buttons[i]), buttton_label);
    gtk_grid_attach(GTK_GRID(grid), buttons[i], i, 0, 1, 1);
    g_signal_connect(buttons[i], "clicked", G_CALLBACK(button_press_cb), NULL);
  }

  /* [> label <] */
  /* GtkWidget *hintlabel = gtk_label_new("H"); */
  /* gtk_widget_add_css_class(hintlabel, "hintlabel"); */
  /* gtk_widget_set_halign(hintlabel, GTK_ALIGN_START); */
  /* gtk_grid_attach(GTK_GRID(grid), hintlabel, 0, 1, 1, 1); */

  gtk_window_present(GTK_WINDOW(window));
  init_overlay(GTK_WINDOW(window));
}

/* rekurencyjnie instaluj dostawcy css */
void apply_css_rec(GtkWidget *root, GtkCssProvider *provider) {
  const char *name = gtk_widget_get_name(root);
  gtk_demo_info("adding css to %s", name);
  add_provider_to_widget(root, provider);
  GtkWidget *child = gtk_widget_get_first_child(root);
  while (child != NULL) {
    apply_css_rec(child, provider);
    child = gtk_widget_get_next_sibling(child);
  }
}

void print_tree_rec(GtkWidget *widget, int rec) {
  char pad[128];
  int padlen = 2 * rec;
  memset(pad, ' ', padlen);
  pad[padlen] = '\0';
  gtk_demo_info("%s%s\n", pad, gtk_widget_get_name(widget));
  GtkWidget *child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    print_tree_rec(child, rec + 1);
    child = gtk_widget_get_next_sibling(child);
  }
}
int main(int argc, char *argv[]) {
  int status;
  if (getenv("TREE") != NULL) {
    print_trees = 1;
  }

  app = gtk_application_new("org.gtk.demo", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate_cb), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
