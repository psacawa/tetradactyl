# dashboard threads -output /dev/null
dashboard assembly -output /dev/null
dashboard registers -output /dev/null
dashboard stack -style limit 15

define interactive-gtk4
set environment GTK_DEBUG=interactive
end

define preload-gtk4
set environment LD_PRELOAD=./build/lib/libtetradactyl-gtk4.so
add-symbol-file ./build/lib/libtetradactyl-gtk4.so
end

define debug-gtk4
set environment LD_LIBRARY_PATH=/usr/local/libd
end

define print_G_OBJECT_TYPE_NAME
  print (g_type_name ((((((GTypeClass*) (((GTypeInstance*) ($arg0))->g_class))->g_type)))))
end
# document G_OBJECT_TYPE_NAME

define preload-qt5
set environment LD_PRELOAD=./build/lib/libtetradactyl-qt5.so
add-symbol-file ./build/lib/libtetradactyl-qt5.so
end

define preload-qt6
set environment LD_PRELOAD=./build/lib/libtetradactyl-qt6.so
add-symbol-file ./build/lib/libtetradactyl-qt6.so
end
