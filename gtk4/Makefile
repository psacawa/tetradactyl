compile:
	meson compile -C build

%.o: %.c
	gcc -c -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)

other-gtk-demo: other-gtk-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gtk4)

gobject-demo: gobject-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gobject-2.0)

gio-demo: gio-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs gio-2.0)

glib-demo: glib-demo.c
	gcc -g $< -o $@ $(shell pkg-config --cflags --libs glib-2.0)

run:
	LD_PRELOAD=./build/libtetradactyl-gtk.so ./build/gtk-demo	

debug:
	gdb -x .gdbinit.debug ./build/gtk-demo


interactive:
	LD_PRELOAD=./build/libtetradactyl-gtk.so GTK_DEBUG=interactive ./build/gtk-demo	

.PHONY: run interactive
