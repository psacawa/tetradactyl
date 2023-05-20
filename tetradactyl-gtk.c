#include <gtk/gtk.h>
#include <stdio.h>

void __attribute__((constructor)) init_tetradactyl() { printf("hello\n"); }
