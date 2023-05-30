dashboard threads -output /dev/null
dashboard assembly -output /dev/null
dashboard registers -output /dev/null
dashboard stack -style limit 20

define interactive
set environment GTK_DEBUG=interactive
end

define preload
set environment LD_PRELOAD=./libtetradactyl-gtk.so
end

define print_G_OBJECT_TYPE_NAME
  print (g_type_name ((((((GTypeClass*) (((GTypeInstance*) ($arg0))->g_class))->g_type)))))
end
# document G_OBJECT_TYPE_NAME
