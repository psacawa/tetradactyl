#ifndef TETRADACTYL_H
#define TETRADACTYL_H value

#include <gtk/gtk.h>

#define ARRAY_LEN(arr) sizeof(arr) / sizeof(arr[0])

#define DECLARE_ORIG_LOC_PTR_EXTERN(sym) extern typeof((sym)) *orig_##sym;
#define DEFINE_ORIG_LOC_PTR(sym) typeof((sym)) *orig_##sym;
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

#define PUBLIC __attribute__((visibility("default")))

/* DECLARATIONS */

/* obrzydliwy wzór c spotykany choćby w glibc */
/* nie da się tego włożyć do nagłówki ze względu na problem wielu definicji */
#define MACRO DECLARE_ORIG_LOC_PTR_EXTERN
#include "replaced-symbols"
#undef MACRO

extern GtkApplication *gtk_app;
// TODO 31/05/20 psacawa: once window management is better understood, we can
// consider supporting multiple windows
extern GtkWindow *main_window;

typedef struct hintiter hintiter;

void hint_overlay_for_active_window();
void clear_hints_for_active_window();
gboolean follow_hint(guint keyval, GdkKeyEvent *event);
void hint_overlay_rec(GtkOverlay *overlay, GtkWidget *widget, hintiter *iter,
                      GArray *hint_widgets);

gboolean key_pressed_cb(GtkEventController *controller, guint keyval,
                        guint keycode, GdkModifierType state);
GtkOverlay *get_active_overlay(GtkApplication *app);
void init_tetradactyl_key_capture(GtkWindow *window);
void init_tetradactyl_overlay_for_active_window();
void init_css();
void clear_tetradactyl_overlay_for_active_window();
void update_tetradactyl_key_capture(GtkApplication *app);

#endif /* ifndef TETRADACTYL_H */
