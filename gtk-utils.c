#include "gtk-utils.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#define INDENT 4

static void print_tree_rec(GtkWidget *widget, int rec);
static void print_gobject_types_tree_rec(GType type, gboolean show_signals,
                                         unsigned int depth);
static gboolean generic_event_cb(GtkEventController *ctrl, GdkEvent *event);

void print_tree(GtkWidget *widget) { print_tree_rec(widget, 0); }

static void print_tree_rec(GtkWidget *widget, int rec) {
  char pad[128];
  int padlen = INDENT * rec;
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

static gboolean generic_event_cb(GtkEventController *ctrl, GdkEvent *event) {
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

void print_gobject_types_tree_from_root(GType root, gboolean show_signals) {
  print_gobject_types_tree_rec(root, show_signals, 0);
}
void print_gobject_types_tree(gboolean show_signals) {
  print_gobject_types_tree_from_root(G_TYPE_OBJECT, show_signals);
}
static void print_gobject_types_tree_rec(GType type, gboolean show_signals,
                                         unsigned int depth) {
  guint n_children;
  char line[128];
  unsigned padlen = INDENT * depth;
  memset(line, ' ', padlen);
  line[padlen] = '\0';
  char signalbuf[256] = "";
  if (show_signals) {
    guint numids;
    guint *sigids = g_signal_list_ids(type, &numids);
    for (int i = 0; i != numids; ++i) {
      GSignalQuery query;
      memset(&query, 0, sizeof(GSignalQuery));
      /* query.signal_id = sigids[i]; */
      g_signal_query(sigids[i], &query);
      strcat(signalbuf, query.signal_name);
      if (i != numids - 1) {
        strcat(signalbuf, ",");
      }
    }
  }
  printf("%s%s %s\n", line, g_type_name(type), signalbuf);
  GType *children = g_type_children(type, &n_children);
  for (int i = 0; i != n_children; ++i) {
    print_gobject_types_tree_rec(children[i], show_signals, depth + 1);
  }
}
