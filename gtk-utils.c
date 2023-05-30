#include "gtk-utils.h"
#include <glib.h>
#include <gtk/gtk.h>

void print_tree_rec(GtkWidget *widget, int rec) {
  char pad[128];
  int padlen = 2 * rec;
  memset(pad, ' ', padlen);
  pad[padlen] = '\0';
  g_info("%s%s\n", pad, gtk_widget_get_name(widget));
  GtkWidget *child = gtk_widget_get_first_child(widget);
  while (child != NULL) {
    print_tree_rec(child, rec + 1);
    child = gtk_widget_get_next_sibling(child);
  }
}

void add_generic_controllers_rec(GtkWidget *root) {
  for (int phase = 1; phase <= 3; ++phase) {
    /* key-pressed */
    GtkEventController *controller = gtk_event_controller_legacy_new();
    gtk_event_controller_set_propagation_phase(controller, phase);
    g_signal_connect_object(controller, "event", G_CALLBACK(generic_event_cb),
                            NULL, 0);
    gtk_widget_add_controller(root, controller);
  }
  GtkWidget *child = gtk_widget_get_first_child(root);
  while (child != NULL) {
    add_generic_controllers_rec(child);
    child = gtk_widget_get_next_sibling(child);
  }
}

gboolean generic_event_cb(GtkEventController *ctrl, GdkEvent *event) {
  GtkWidget *widget = gtk_event_controller_get_widget(ctrl);
  GtkPropagationPhase phase = gtk_event_controller_get_propagation_phase(ctrl);
  printf("%8s %d %s\n",
         phase == GTK_PHASE_CAPTURE  ? "CAPTURE"
         : phase == GTK_PHASE_TARGET ? "TARGET"
         : phase == GTK_PHASE_BUBBLE ? "BUBBLE"
                                     : "unknown",
         gdk_event_get_event_type(event), gtk_widget_get_name(widget));
  /* return phase == GTK_PHASE_TARGET; */
  return FALSE;
}
