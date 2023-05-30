#include <gtk/gtk.h>

#define print_tree(w) print_tree_rec(w, 0)
void print_tree_rec(GtkWidget *widget, int rec);

gboolean generic_event_cb(GtkEventController *ctrl, GdkEvent *event);
