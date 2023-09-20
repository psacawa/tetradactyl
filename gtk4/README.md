<!-- Copyright 2023 Paweł Sacawa. All rights reserved. -->
# Tetradactyl GTK

Tetradactyl backend for gtk4.

## Compile

1. Install `meson`, `ninja`, `libgtk4`
2. `meson setup build`
3. `meson compile -C build`

## Running

`make run`, which is:

```
LD_PRELOAD=./build/libtetradactyl-gtk.so  ./build/gtk-demo
```

- f: Hint
- Esc: Clear hints
