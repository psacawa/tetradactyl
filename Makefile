all: libtetradactyl-gtk.so tetradactyl-gtk-demo gtk-demo gobject-demo gio-demo glib-demo

%.o: %.c
	gcc -c -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)

gtk-demo: gtk-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)

gobject-demo: gobject-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gobject-2.0)

gio-demo: gio-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gio-2.0)

glib-demo: glib-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs glib-2.0)

run:
	LD_PRELOAD=./build/libtetradactyl-gtk.so  ./build/gtk-demo	

interactive:
	GTK_DEBUG=interactive ./gtk-demo	

.PHONY: run interactive
