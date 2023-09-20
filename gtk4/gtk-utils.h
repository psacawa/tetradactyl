// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <gtk/gtk.h>

void print_tree(GtkWidget *widget);

void print_gobject_types_tree_from_root(GType root, gboolean show_signals);
void print_gobject_types_tree(gboolean show_signals);
