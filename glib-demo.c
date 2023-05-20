#include <glib.h>

int main(int argc, char *argv[]) {
  GMainContext *context = g_main_context_new();
  GMainLoop *loop = g_main_loop_new(context, FALSE);
  g_main_loop_run(loop);
}
