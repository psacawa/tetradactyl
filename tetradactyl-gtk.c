#include <ctype.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LEN(arr) sizeof(arr) / sizeof(arr[0])

#define DECLARE_ORIG_LOC_PTR(sym) static typeof((sym)) *orig_##sym;
#define SYMBOL_MAP_ENTRY(sym) {(#sym), &(orig_##sym)},
#define orig(sym) orig##(sym)

#define TETRADACTYL_HINT_KEY GDK_KEY_f
#define TETRADACTYL_FOLLOW_KEY GDK_KEY_f
#define TETRADACTYL_ESCAPE_KEY GDK_KEY_Escape

#define tetradactyl_debug(...)                                                 \
  g_log("tetradactyl", G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define tetradactyl_info(...)                                                  \
  g_log("tetradactyl", G_LOG_LEVEL_INFO, __VA_ARGS__)
#define tetradactyl_warning(...)                                               \
  g_log("tetradactyl", G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define tetradactyl_error(...)                                                 \
  g_log("tetradactyl", G_LOG_LEVEL_ERROR, __VA_ARGS__)
#define tetradactyl_critical(...)                                              \
  g_log("tetradactyl", G_LOG_LEVEL_CRITICAL, __VA_ARGS__)

/* DECLARATIONS */

/* obrzydliwy wzór c spotykany choćby w glibc */
#define MACRO DECLARE_ORIG_LOC_PTR
#include "replaced-symbols"
#undef MACRO

typedef struct hintiter hintiter;

static void hint_overlay_for_active_window();
static void clear_hints_for_active_window();
static gboolean follow_hint(guint keyval, GdkKeyEvent *event);
static void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget,
                             hintiter *iter, GArray *hint_widgets);

static gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                               guint keycode, GdkModifierType state);
static GtkOverlay *get_active_overlay(GtkApplication *app);
static void init_tetradactyl_key_capture(GtkApplication *app);
static void init_tetradactyl_overlay_for_active_window();
static void init_css();
static void clear_tetradactyl_overlay_for_active_window();

/* DEFINITIONS */

typedef struct hintiter {
  unsigned int hintidx;
} hintiter;

/* na razie globalny stan */
enum {
  TETRADACTYL_MODE_NORMAL = 0,
  TETRADACTYL_MODE_HINT,
} tetradactyl_mode = TETRADACTYL_MODE_NORMAL;

/* ponieważ okna są wejściem do przechwytania wydarzeń musimy replikować zbiór
 * okien z libgtk w tetradactyl */
static GList *apps = NULL;
static GList *windows = NULL;

static const char *hintchars = "ASDFGHJKLQWERTYUIOPZXCVBNM";
#define NUMHINTS strlen(hintchars)

static int ishint_keyval(guint keyval) {
  /* TODO 27/05/20 psacawa: more robust */
  return (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z) ||
         (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z);
}

static int is_hintable(GtkWidget *widget) { return GTK_IS_BUTTON(widget); }

static char *hint_next(hintiter *iter) {
  char *ret = malloc(2);
  if (iter->hintidx < strlen(hintchars)) {
    ret[0] = hintchars[iter->hintidx++];
    ret[1] = '\0';
  } else {
    /* dwuliterowe podpowiedzi */
  }
  return ret;
}

static GtkOverlay *get_active_overlay(GtkApplication *app) {
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

static symbol_map_entry_t map[] = {
/* {"gtk_init", &orig_gtk_init}, */
#define MACRO SYMBOL_MAP_ENTRY
#include "replaced-symbols"
#undef MACRO
};

static void __attribute__((constructor)) init_gtk_proxy() {
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
}

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

static gboolean get_child_position_cb(GtkWidget *overlay, GtkWidget *hint,
                                      GdkRectangle *allocation,
                                      gpointer user_data) {
  tetradactyl_info(
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

static gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                               guint keycode, GdkModifierType state) {

  GdkKeyEvent *orig_event =
      (GdkKeyEvent *)(gtk_event_controller_get_current_event(controller));
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
    }
  } else if (tetradactyl_mode == TETRADACTYL_MODE_HINT) {
    if (keyval == TETRADACTYL_ESCAPE_KEY) {
      clear_hints_for_active_window();
      clear_tetradactyl_overlay_for_active_window();
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

static void hint_overlay_for_active_window() {
  GtkOverlay *overlay = get_active_overlay(apps->data);

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

static void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget,
                             hintiter *iter, GArray *hint_widgets) {

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

static void clear_hints_for_active_window() {
  GtkOverlay *overlay = get_active_overlay(apps->data);
  GArray *hint_widgets = g_object_get_data(G_OBJECT(overlay), "hint-widgets");
  for (int i = 0; i != hint_widgets->len; ++i) {
    GtkWidget *widget = g_array_index(hint_widgets, GtkWidget *, i);
    gtk_overlay_remove_overlay(overlay, widget);
  }
  tetradactyl_mode = TETRADACTYL_MODE_NORMAL;
}

static void send_synthetic_click_event(GtkWidget *hint) {
  GtkWindow *win = gtk_application_get_active_window(apps->data);
  GtkButton *button = g_object_get_data(G_OBJECT(hint), "target");
  gtk_widget_activate(GTK_WIDGET(button));
}

/* zwróc czy stosowna podpowiedź została odnaleziona */
static gboolean follow_hint(guint keyval, GdkKeyEvent *orig_event) {
  tetradactyl_info(__FUNCTION__);
  g_assert(tetradactyl_mode == TETRADACTYL_MODE_HINT);
  GtkOverlay *overlay = get_active_overlay(apps->data);
  GArray *hint_widgets = g_object_get_data(G_OBJECT(overlay), "hint-widgets");
  for (int i = 0; i != hint_widgets->len; ++i) {
    GtkWidget *hint = g_array_index(hint_widgets, GtkWidget *, i);
    const char *hintlabel = gtk_label_get_text(GTK_LABEL(hint));
    if (hintlabel[0] == keyval) {
      tetradactyl_info("following hint with label %c", keyval);
      send_synthetic_click_event(hint);
      clear_hints_for_active_window();
      return TRUE;
    }
  }
  return FALSE;
}

/* Check whether libgtk has created any new windows. If so, add the Tetradactyl
 * keypress event controller to them. To called up every exit from an
 * intercepted libgtk routine. Hot path must be fast. */
static void update_tetradactyl_key_capture(GtkApplication *app) {
  GList *app_windows = gtk_application_get_windows(app);
  while (app_windows != NULL) {
    GtkWindow *win = app_windows->data;
    /* oddly, gkt_main_do_event will create a GtkTooltip(Window) on the first
     * event  */
    /* FIXME 31/05/20 psacawa: implement properly */
    if (!GTK_IS_WINDOW(win) && g_list_find(windows, win) == NULL) {
      windows = g_list_append(windows, app_windows->data);
      init_tetradactyl_key_capture(app_windows->data);
    }
  }
}

/* capture */
static void init_tetradactyl_key_capture(GtkApplication *application) {
  tetradactyl_info("initalizing tetradactyl key capture on window");
  GtkWindow *window = gtk_application_get_active_window(application);
  if (!window) {
    tetradactyl_error("no active window for application");
  }
  GtkEventController *key_capture_controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(key_capture_controller,
                                             GTK_PHASE_CAPTURE);
  g_signal_connect_object(key_capture_controller, "key-pressed",
                          G_CALLBACK(key_pressed_cb), NULL, 0);
  gtk_event_controller_set_name(key_capture_controller,
                                "tetradactyl-controller");
  gtk_widget_add_controller(GTK_WIDGET(window), key_capture_controller);
}

static void init_tetradactyl_overlay_for_active_window() {
  /* TODO 30/05/20 psacawa: one app? */
  GtkWindow *window = gtk_application_get_active_window(apps->data);
  GtkWidget *child = gtk_window_get_child(window);
  tetradactyl_debug("initializing tetradactyl");
  GtkWidget *overlay = gtk_overlay_new();
  gtk_window_set_child(window, overlay);
  gtk_overlay_set_child(GTK_OVERLAY(overlay), child);
  g_signal_connect(overlay, "get-child-position",
                   G_CALLBACK(get_child_position_cb), NULL);
}

static void init_css() {
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_path(provider, "./hints.css");
  GdkDisplay *display = gdk_display_get_default();
  gtk_style_context_add_provider_for_display(display,
                                             GTK_STYLE_PROVIDER(provider), 0);
}

static void clear_tetradactyl_overlay_for_active_window() {
  GtkWindow *window = gtk_application_get_active_window(apps->data);
  GtkWidget *overlay = gtk_window_get_child(window);
  if (!GTK_IS_OVERLAY(overlay)) {
    tetradactyl_error(
        "clearing tetradactyl overlay, but window child was not a GtkOverlay");
    return;
  }
  GtkWidget *child = gtk_overlay_get_child(GTK_OVERLAY(overlay));
  gtk_window_set_child(window, child);
}
