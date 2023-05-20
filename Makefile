all: libtetradactyl-gtk.so gtk-demo

libtetradactyl-gtk.so: ./tetradactyl-gtk.c
	gcc -g -shared -fPIC $< -o $@ $(shell pkg-config --cflags --libs gtk4)

%: %.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)

glib-demo: glib-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs glib-2.0)


run:
	LD_PRELOAD=./libtetradactyl-gtk.so  ./gtk-demo	
