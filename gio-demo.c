#include <gio/gio.h>
#include <stdio.h>

void greet_cb(GSimpleAction *action, GVariant *param, gpointer data) {
  const gchar *name = g_variant_get_string(param, 0);
  printf("hello %s!\n", name);
}

void flick_cb(GSimpleAction *action, GVariant *param, gpointer data) {
  /* printf("%p %p \n", param, data); */
  gboolean light = g_variant_get_boolean(g_action_get_state(G_ACTION(action)));
  light = !light;
  printf("flicked light %s\n", light ? "on" : "off");
  g_simple_action_set_state(action, g_variant_new_boolean(light));
}

int main(int argc, char *argv[]) {
  GAction *greet_action =
      G_ACTION(g_simple_action_new("greet", g_variant_type_new("s")));
  g_signal_connect(greet_action, "activate", G_CALLBACK(greet_cb), NULL);
  GVariant *g_name = g_variant_new("s", "paweł");

  GAction *flick_action = G_ACTION(g_simple_action_new_stateful(
      "flick", NULL, g_variant_new_boolean(FALSE)));
  g_signal_connect(flick_action, "activate", G_CALLBACK(flick_cb), NULL);

  GSimpleActionGroup *group = g_simple_action_group_new();
  g_action_map_add_action(G_ACTION_MAP(group), flick_action);
  g_action_map_add_action(G_ACTION_MAP(group), greet_action);

  /* g_action_activate(greet_action, g_name); */
  /* for (int i = 0; i != 5; ++i) { */
  /*   g_action_activate(flick_action, NULL); */
  /* } */

  g_action_group_activate_action(G_ACTION_GROUP(group), "flick", NULL);

  for (char **name = g_action_group_list_actions(G_ACTION_GROUP(group));
       *name != NULL; name++) {
    printf("%s\n", *name);
  }
}
