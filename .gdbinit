# Copyright 2023 Paweł Sacawa. All rights reserved.
# dashboard threads -output /dev/null
# dashboard assembly -output /dev/null
# dashboard registers -output /dev/null
# dashboard stack -style limit 15

# GTK

define interactive-gtk4
  set environment GTK_DEBUG=interactive
  dont-repeat
end

define preload-gtk4
  set environment LD_PRELOAD=./build/lib/libtetradactyl-gtk4.so
  add-symbol-file ./build/lib/libtetradactyl-gtk4.so -readnow
  dont-repeat
end

define debug-gtk4
set environment LD_LIBRARY_PATH=/usr/local/libd
dont-repeat
end

define print_G_OBJECT_TYPE_NAME
  print (g_type_name ((((((GTypeClass*) (((GTypeInstance*) ($arg0))->g_class))->g_type)))))
end
# document G_OBJECT_TYPE_NAME

# QT

define preload-qt5
  set environment LD_PRELOAD=./build/lib/libtetradactyl-qt5.so
  add-symbol-file ./build/lib/libtetradactyl-qt5.so -readnow
  dont-repeat
end

define preload-qt6
  set environment LD_PRELOAD=./build/lib/libtetradactyl-qt6.so
  add-symbol-file ./build/lib/libtetradactyl-qt6.so -readnow
  dont-repeat
end

define preload-dynamic
  set environment LD_PRELOAD=./build/lib/libtetradactyl-dynamic-probe.so
  add-symbol-file ./build/lib/libtetradactyl-dynamic-probe.so -readnow
  dont-repeat
end

define offscreen-qt
  set environment QT_QPA_PLATFORM=offscreen
  dont-repeat
end

define printMeta
  print $arg0->metaObject()
end
