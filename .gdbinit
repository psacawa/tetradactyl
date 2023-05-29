dashboard threads -output no

define print_G_OBJECT_TYPE_NAME
  print (g_type_name ((((((GTypeClass*) (((GTypeInstance*) ($arg0))->g_class))->g_type)))))
end
# document G_OBJECT_TYPE_NAME
