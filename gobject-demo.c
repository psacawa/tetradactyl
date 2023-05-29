#include <glib-object.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_TYPE_GREETER app_greeter_get_type()
G_DECLARE_DERIVABLE_TYPE(AppGreeter, app_greeter, APP, GREETER, GObject);

typedef struct _AppGreeterClass AppGreeterClass;
struct _AppGreeterClass {
  GObjectClass parent_class;
  void (*greet)(AppGreeter *self, const char *name);
  void (*print_summary)(AppGreeter *self);
};

typedef struct _AppGreeterPrivate {
  int times_called;
  GHashTable *people_greeted;
  const char *greeting;
} AppGreeterPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(AppGreeter, app_greeter, G_TYPE_OBJECT);

gboolean str_equal(gconstpointer str1, gconstpointer str2) {
  return !g_strcmp0(str1, str2);
}

void app_greeter_init(AppGreeter *self) {
  AppGreeterPrivate *priv = app_greeter_get_instance_private(self);
  priv->times_called = 0;
  priv->people_greeted = g_hash_table_new(g_str_hash, str_equal);
}

AppGreeter *app_greeter_new(const char *greeting) {
  printf("app_greeter_new\n");
  AppGreeter *self = g_object_new(APP_TYPE_GREETER, NULL);
  AppGreeterPrivate *priv = app_greeter_get_instance_private(self);
  priv->greeting = g_strdup(greeting);
  return self;
}

void app_greeter_greet(AppGreeter *self, const char *name) {
  AppGreeterPrivate *priv = app_greeter_get_instance_private(self);
  printf("%s %s\n", priv->greeting, name);
  priv->times_called++;
  GHashTable *ht = priv->people_greeted;
  int times_greeted = GPOINTER_TO_INT(g_hash_table_lookup(ht, name));
  g_hash_table_replace(ht, (gpointer)name, GINT_TO_POINTER(times_greeted + 1));
}

void ht_foreach_printf(gpointer key, gpointer value, gpointer user_data) {
  printf("greeted %s %d times \n", (char *)key, GPOINTER_TO_INT(value));
}

void app_greeter_print_summary(AppGreeter *self) {
  AppGreeterPrivate *priv = app_greeter_get_instance_private(self);
  printf("called %d times\n", priv->times_called);
  g_hash_table_foreach(priv->people_greeted, ht_foreach_printf, NULL);
}

void app_greeter_constructed(AppGreeter *self) {
  AppGreeterPrivate *priv = app_greeter_get_instance_private(self);
  priv->times_called = 0;
};

void app_greeter_class_init(AppGreeterClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  /* object_class->constructed = app_greeter_constructed; */
  class->greet = app_greeter_greet;
  class->print_summary = app_greeter_print_summary;
}

int main(int argc, char *argv[]) {
  AppGreeter *greeter = app_greeter_new("dzień dobry");
  app_greeter_greet(greeter, "paweł");
  app_greeter_greet(greeter, "piotr");
  app_greeter_greet(greeter, "piotr");
  app_greeter_print_summary(greeter);
  GVariant *var =
      g_variant_parse(G_VARIANT_TYPE_INT32, "-123", NULL, NULL, NULL);
  printf("%s\n", g_variant_get_type_string(var));
  gint64 val = g_variant_get_int32(var);
  printf("%ld\n", val);
}
