#include <ctype.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LEN(arr) sizeof(arr) / sizeof(arr[0])

#define DECLARE_ORIG_LOC_PTR(sym) typeof((sym)) *orig_##sym;
#define SYMBOL_MAP_ENTRY(sym) {(#sym), &(orig_##sym)},

/* obrzydliwy wzór c spotykany choćby w glibc */
#define MACRO DECLARE_ORIG_LOC_PTR
#include "replaced-symbols"
#undef MACRO

#define TETRADACTYL_HINT_KEY GDK_KEY_f
#define TETRADACTYL_FOLLOW_KEY GDK_KEY_f
#define TETRADACTYL_ESCAPE_KEY GDK_KEY_Escape

#define gtk_demo_debug(...) g_log("gtk-demo", G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define gtk_demo_info(...) g_log("gtk-demo", G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define gtk_demo_warning(...)                                                  \
  g_log("gtk-demo", G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define gtk_demo_error(...) g_log("gtk-demo", G_LOG_LEVEL_ERROR, __VA_ARGS__)

enum {
  TETRADACTYL_MODE_NORMAL = 0,
  TETRADACTYL_MODE_HINT,
} tetradactyl_mode = TETRADACTYL_MODE_NORMAL;

GtkApplication *app;

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

/* SYMBOL_MAP_ENTRY(gtk_init), SYMBOL_MAP_ENTRY(gtk_application_new), */
/* SYMBOL_MAP_ENTRY(gtk_window_new)}; */

void __attribute__((constructor)) init_gtk_proxy() {
  for (int i = 0; i != ARRAY_LEN(map); ++i) {
    void *org_symbol_loc = dlsym(RTLD_NEXT, map[i].symbol_name);
    if (org_symbol_loc == NULL) {
      fprintf(stderr, "symbol %s not loaded: %s\n", map[i].symbol_name,
              dlerror());
      exit(EXIT_FAILURE);
    }
    *(void **)(map[i].ptr_to_ptr_to_symbol_loc) = org_symbol_loc; /* ? */
  }
}

void gtk_init() {
  printf("in gtk_init\n");
  orig_gtk_init();
  printf("returning from  gtk_init\n");
}

GtkApplication *gtk_application_new(const gchar *application_id,
                                    GApplicationFlags flags) {
  printf("in gtk_application_new %p \n", orig_gtk_application_new);
  app = (*orig_gtk_application_new)(application_id, flags);
  printf("returning from gtk_application_new\n");
  return app;
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
    /* escape -> kill hints */
    /* tab -> iterate hints */
    /* alpha -> select hints */
    /* enter -> actvate hint */
  }
  return FALSE;
}

/* Check whether libgtk has created any new windows. If so, add the Tetradactyl
 * keypress event controller to them. To called up every exit from an
 * intercepted libgtk routine. Hot path must be fast. */
void update_tetradactyl_key_capture() {

}

/* capture */
void init_tetradactyl_key_capture(GtkWindow *window) {
  GtkEventController *key_capture_controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(key_capture_controller,
                                             GTK_PHASE_CAPTURE);
  g_signal_connect_object(key_capture_controller, "key-pressed",
                          G_CALLBACK(key_pressed_cb), NULL, 0);
  gtk_event_controller_set_name(key_capture_controller,
                                "tetradactyl-controller");
  gtk_widget_add_controller(GTK_WIDGET(window), key_capture_controller);
}

void init_tetradactyl_overlay(GtkWindow *window) {

  GtkWidget *child = gtk_window_get_child(window);
  gtk_demo_debug("initializing tetradactyl");
  GtkWidget *overlay = gtk_overlay_new();
  gtk_window_set_child(window, overlay);
  gtk_overlay_set_child(GTK_OVERLAY(overlay), child);

  g_signal_connect(overlay, "get-child-position",
                   G_CALLBACK(get_child_position_cb), NULL);
}
