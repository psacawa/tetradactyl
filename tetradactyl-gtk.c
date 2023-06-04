#define G_LOG_USE_STRUCTURED

#include <ctype.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtk-utils.h"
#include "tetradactyl.h"

/* DEFINITIONS */

typedef struct hintiter {
  unsigned required;
  /* can't compute the hints in an online manner without already knowing the
   * number required */
  unsigned hintlen;
  unsigned int hintidx;
} hintiter;

tetradactyl_config tconfig;

/* na razie globalny stan */
enum {
  TETRADACTYL_MODE_NORMAL = 0,
  TETRADACTYL_MODE_HINT = 1,
} tetradactyl_mode = TETRADACTYL_MODE_NORMAL;

/* ponieważ okna są wejściem do przechwytania wydarzeń musimy replikować zbiór
 * okien z libgtk w tetradactyl */
GtkApplication *gtk_app = NULL;
GtkWindow *main_window = NULL;
GList *cached_windows = NULL;

int ishint_keyval(guint keyval) {
  /* TODO 27/05/20 psacawa: more robust */
  return (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) ||
         (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z);
}

int is_hintable(GtkWidget *widget) {
  return GTK_IS_BUTTON(widget) || GTK_IS_CHECK_BUTTON(widget) ||
         GTK_IS_TOGGLE_BUTTON(widget);
}

hintiter *hintiter_init(unsigned required) {
  hintiter *ret = malloc(sizeof(hintiter));
  memset(ret, 0, sizeof(hintiter));
  ret->required = required;
  for (; required; required /= strlen(tconfig.hintchars)) {
    ret->hintlen++;
  }
  return ret;
}

/* for "ASDF" yields A S D F AA AS ... FD FF AAA ... */
static char *hintiter_next(hintiter *iter) {
  /* using hintchars as digits, represent iter->hintidx in radix form */
  size_t hintchars_len = strlen(tconfig.hintchars);
  char *hintstr = malloc(iter->hintlen + 1);

  unsigned idx = iter->hintidx;
  for (int i = iter->hintlen - 1; i >= 0; --i) {
    char ch = tconfig.hintchars[idx % hintchars_len];
    hintstr[i] = toupper(ch);
    idx /= hintchars_len;
  }
  hintstr[iter->hintlen] = '\0';
  iter->hintidx++;
  return hintstr;
}

GtkOverlay *get_active_overlay(GtkApplication *app) {
  GtkWindow *window = gtk_application_get_active_window(app);
  GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(window));
  while (child) {
    if (GTK_IS_OVERLAY(child)) {
      return GTK_OVERLAY(child);
    }
    child = gtk_widget_get_next_sibling(child);
  }
  tetradactyl_error("active window of type %s had no GtkOverlay child",
                    gtk_widget_get_name(GTK_WIDGET(window)));
  return NULL;
}

#define MACRO DEFINE_ORIG_LOC_PTR
#include "replaced-symbols"
#undef MACRO

typedef struct symbol_map_entry_t {
  char *symbol_name;
  void *ptr_to_ptr_to_symbol_loc;
  /* wskaźnik do położenia w libgtk-proxy gdzie przechowane jest zmienna orig_*
   * która sama jest wskaźnikiem do libgtk-4 */
} symbol_map_entry_t;

symbol_map_entry_t map[] = {
/* {"gtk_init", &orig_gtk_init}, */
#define MACRO SYMBOL_MAP_ENTRY
#include "replaced-symbols"
#undef MACRO
};

void __attribute__((constructor)) init_gtk_proxy() {
  /* setup proxy function pointers */
  for (int i = 0; i != ARRAY_LEN(map); ++i) {
    void *org_symbol_loc = dlsym(RTLD_NEXT, map[i].symbol_name);
    if (org_symbol_loc == NULL) {
      tetradactyl_error("symbol %s not loaded: %s\n", map[i].symbol_name,
                        dlerror());
      exit(EXIT_FAILURE);
    }
    *(void **)(map[i].ptr_to_ptr_to_symbol_loc) = org_symbol_loc; /* ? */
  }

  /* other early setup */
  memset(&tconfig, 0, sizeof(tetradactyl_config));
  /* config from json? */
  /* tconfig.hintchars = "asdfghjkl"; */
  tconfig.hintchars = "asd";
}

gboolean get_child_position_cb(GtkWidget *overlay, GtkWidget *hint,
                               GdkRectangle *allocation, gpointer user_data) {
  tetradactyl_info(
      "get_child_position_cb: widget:%s x=%d y=%d width=%d height=%d\n",
      gtk_widget_get_name(hint), allocation->x, allocation->y,
      allocation->width, allocation->height);

  GtkWidget *target = g_object_get_data(G_OBJECT(hint), "target");
  int natural;
  /* longer hints need horizontal allocation */
  gtk_widget_measure(hint, GTK_ORIENTATION_HORIZONTAL, 0, NULL, &natural, NULL,
                     NULL);
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
  allocation->width = natural;
  allocation->height = 20;
  return TRUE;
}

gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                        guint keycode, GdkModifierType state) {
  /* keycode odzwierciedla fizyczny klawisz i nie jest interesujący z
   * perspektywy naszej aplikacji */
  tetradactyl_debug("key_pressed_cb: "
                    "keyval=%#x=%d=%c "
                    "state=%d",
                    keyval, keyval, keyval, state);
  if (tetradactyl_mode == TETRADACTYL_MODE_NORMAL) {
    if (keyval == TETRADACTYL_HINT_KEY &&
        (state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_ALT_MASK)) == 0) {
      tetradactyl_debug("rendering hints");
      init_tetradactyl_overlay_for_active_window();
      hint_overlay_for_active_window();
      return TRUE;
    } else if (keyval == 't') {
      print_gobject_types_tree(TRUE);
    }
  } else if (tetradactyl_mode == TETRADACTYL_MODE_HINT) {
    if (keyval == TETRADACTYL_ESCAPE_KEY) {
      clear_hints_for_active_window();
      clear_tetradactyl_overlay_for_active_window();
    } else if (ishint_keyval(keyval)) {
      filter_hints(keyval);
    }

    /* in hint mode, tetradactyl eats all key presses */
    return TRUE;
  }
  return FALSE;
}

void hint_overlay_for_active_window() {
  GtkOverlay *overlay = get_active_overlay(gtk_app);

  /* spis elementów na overlay, skoro gtk nie udostępnia te dane?? */
  GArray *hint_labels = g_object_get_data(G_OBJECT(overlay), "hint-labels");
  if (hint_labels != NULL) {
    g_array_free(hint_labels, TRUE);
  }
  hint_labels = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));

  GArray *hintables = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));

  GtkWidget *root_under_overlay =
      gtk_widget_get_first_child(GTK_WIDGET(overlay));
  get_hintables_for_overlay_rec(overlay, root_under_overlay, hintables);
  hintiter *iter = hintiter_init(hintables->len);

  for (int i = 0; i != hintables->len; ++i) {

    GtkWidget *hintable = g_array_index(hintables, GtkWidget *, i);

    char *hintstr = hintiter_next(iter);
    if (!hintstr) {
      tetradactyl_error("failed to generate string");
      return;
    }
    GtkWidget *hintlabel = gtk_label_new(hintstr);

    g_object_set_data(G_OBJECT(hintlabel), "target", hintable);
    g_object_ref(hintable);

    gtk_widget_add_css_class(hintlabel, "hintlabel");
    gtk_overlay_add_overlay(overlay, hintlabel);
    g_array_append_val(hint_labels, hintlabel);
  }

  g_object_set_data(G_OBJECT(overlay), "hint-labels", hint_labels);
  tetradactyl_mode = TETRADACTYL_MODE_HINT;
}

void get_hintables_for_overlay_rec(GtkOverlay *overlay, GtkWidget *widget,
                                   GArray *hintables) {
  if (is_hintable(widget)) {
    g_array_append_val(hintables, widget);
  }

  /* hintable does not preclude hintable children */
  GtkWidget *child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    get_hintables_for_overlay_rec(overlay, child, hintables);
    child = gtk_widget_get_next_sibling(child);
  }
  return;
}

void clear_hints_for_active_window() {
  GtkOverlay *overlay = get_active_overlay(gtk_app);
  GArray *hint_labels = g_object_get_data(G_OBJECT(overlay), "hint-labels");
  for (int i = 0; i != hint_labels->len; ++i) {
    GtkWidget *widget = g_array_index(hint_labels, GtkWidget *, i);
    gtk_overlay_remove_overlay(overlay, widget);
  }
  tetradactyl_mode = TETRADACTYL_MODE_NORMAL;
}

void send_synthetic_click_event(GtkWidget *hint) {
  GtkButton *button = g_object_get_data(G_OBJECT(hint), "target");
  gtk_widget_activate(GTK_WIDGET(button));
}

void filter_hints(unsigned char keyval) {
  GtkOverlay *overlay = get_active_overlay(gtk_app);
  /* current policy: hints are rendered uppercase */
  keyval = toupper(keyval);

  char *hint_input = g_object_get_data(G_OBJECT(overlay), "hint-input");
  GArray *hint_labels = g_object_get_data(G_OBJECT(overlay), "hint-labels");
  unsigned inplen = strlen(hint_input);
  hint_input[inplen] = keyval;
  hint_input[++inplen] = '\0';

  for (int i = 0; i != hint_labels->len; ++i) {
    GtkWidget *label = g_array_index(hint_labels, GtkWidget *, i);
    const char *label_text = gtk_label_get_text(GTK_LABEL(label));
    if (!strncmp(label_text, hint_input, inplen)) {
      /* input is prefix of hint */
      if (strlen(label_text) == inplen) {
        /* equal */
        send_synthetic_click_event(GTK_WIDGET(label));
        clear_hints_for_active_window();
        clear_tetradactyl_overlay_for_active_window();
        return;
      }
    } else {
      /* input is prefix of hint, hide hint */
      gtk_widget_set_visible(label, FALSE);
    }
  }
}

/* Check whether libgtk has created any new windows. If so, add the
 * Tetradactyl keypress event controller to them. To called up every exit from
 * an intercepted libgtk routine. Hot path must be fast. */
void update_tetradactyl_key_capture(GtkApplication *app) {
  GList *app_windows = gtk_application_get_windows(app);
  if (g_list_length(app_windows) <= 1) {
    if (g_list_length(app_windows) != g_list_length(cached_windows)) {
      /* cold path - search for new window and possibly add controllers */
      while (app_windows) {
        GtkWindow *win = app_windows->data;
        /* oddly, gkt_main_do_event will create a GtkTooltip(Window) on the
         * first event. For now, only create controller on first window */
        /* FIXME 31/05/20 psacawa: implement properly */
        if (g_list_find(cached_windows, win) == NULL) {
          cached_windows = g_list_append(cached_windows, app_windows->data);
          init_tetradactyl_key_capture(app_windows->data);
        }
        app_windows = app_windows->next;
      }
    }
  }

  /* check if display manager has a display. if so, setup css */
  /* can be moved to a colder, earlier path once startup is better understood
   */
  GdkDisplayManager *dm = gdk_display_manager_get();
  GdkDisplay *display = gdk_display_manager_get_default_display(dm);
  if (display) {
    init_css();
  }
}

/* capture */
void init_tetradactyl_key_capture(GtkWindow *window) {
  tetradactyl_info("initalizing tetradactyl key capture on window");
  GtkEventController *key_capture_controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(key_capture_controller,
                                             GTK_PHASE_CAPTURE);
  g_signal_connect_object(key_capture_controller, "key-pressed",
                          G_CALLBACK(key_pressed_cb), NULL, 0);
  gtk_event_controller_set_name(key_capture_controller,
                                "tetradactyl-controller");
  gtk_widget_add_controller(GTK_WIDGET(window), key_capture_controller);
}

void init_tetradactyl_overlay_for_active_window() {
  /* TODO 30/05/20 psacawa: one app? */
  GtkWindow *window = gtk_application_get_active_window(gtk_app);
  GtkWidget *child = gtk_window_get_child(window);
  tetradactyl_debug("initializing tetradactyl");
  GtkWidget *overlay = gtk_overlay_new();
  gtk_window_set_child(window, overlay);
  gtk_overlay_set_child(GTK_OVERLAY(overlay), child);

  char *hint_input = malloc(MAX_HINT_LEN + 1);
  hint_input[0] = '\0';
  g_object_set_data(G_OBJECT(overlay), "hint-input", hint_input);

  g_signal_connect(overlay, "get-child-position",
                   G_CALLBACK(get_child_position_cb), NULL);
}

void init_css() {
  GtkCssProvider *provider = gtk_css_provider_new();
  /* gtk_css_provider_load_from_path(provider, "./hints.css"); */
  gtk_css_provider_load_from_resource(provider, "/org/gtk/tetradactyl/hints.css");

  GdkDisplay *display = gdk_display_get_default();
  tetradactyl_info("display %p", display);
  gtk_style_context_add_provider_for_display(display,
                                             GTK_STYLE_PROVIDER(provider), 0);
}

void clear_tetradactyl_overlay_for_active_window() {
  GtkWindow *window = gtk_application_get_active_window(gtk_app);
  GtkWidget *overlay = gtk_window_get_child(window);
  if (!GTK_IS_OVERLAY(overlay)) {
    tetradactyl_error("clearing tetradactyl overlay, but window child was "
                      "not a GtkOverlay");
    return;
  }
  GtkWidget *child = gtk_overlay_get_child(GTK_OVERLAY(overlay));
  gtk_overlay_set_child(GTK_OVERLAY(overlay), NULL);
  gtk_window_set_child(window, child);
}
