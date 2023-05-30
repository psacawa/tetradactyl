# FIXME 27/05/20 psacawa: this is currently broken
# PKG_CONFIG_PATH=/home/psacawa/Repozytoria/gtk/build-debug/meson-uninstalled
# export PKG_CONFIG_PATH

all: libtetradactyl-gtk.so tetradactyl-gtk-demo gtk-demo gobject-demo gio-demo glib-demo

libtetradactyl-gtk.so: ./tetradactyl-gtk.c
	gcc -g -shared -fPIC $< -o $@ $(shell pkg-config --cflags --libs gtk4)

%: %.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)

gtk-demo: gtk-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)



gobject-demo: gobject-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gobject-2.0)

gio-demo: gio-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gio-2.0)

glib-demo: glib-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs glib-2.0)


run:
	LD_PRELOAD=./libtetradactyl-gtk.so  ./gtk-demo	

interactive:
	GTK_DEBUG=interactive ./gtk-demo	


debug:
	gdb ./gtk-demo -x /dev/stdin <<EOF 
		set environment LD_PRELOAD=./libtetradactyl-gtk.so 
		break main
		run
	EOF

.PHONY: run debug
