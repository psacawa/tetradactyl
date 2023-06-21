project('tetradactyl-gtk', 'c', version: '0.1')

gnome = import('gnome')

gtk4_dep = dependency('gtk4')
pwd_inc = include_directories('.')

# libtetradactyl-gtk.so

tetradactyl_resources = gnome.compile_resources('tetradactyl_resources',
  './tetradactyl.gresource.xml',
  source_dir: meson.current_source_dir()
)

tetradactyl_sources = files([
  'tetradactyl-gtk.c',
  'init.c',
  'intercepted-symbols.c',
  'gtk-utils.c',
])

tetradactyl_headers =  files(['tetradactyl.h'])

shared_library('tetradactyl-gtk',
  sources: tetradactyl_sources + tetradactyl_resources,
  dependencies: [gtk4_dep],
  gnu_symbol_visibility: 'hidden',
  include_directories: pwd_inc,
)

# gtk-demo


gtk_demo_resources = gnome.compile_resources('gtk_demo_resources',
  'gtk-demo.gresource.xml',
  source_dir: meson.current_source_dir()
)

demo_sources = files(['gtk-demo.c'])
executable('gtk-demo', 
  sources: demo_sources + gtk_demo_resources,
  dependencies: [gtk4_dep],
  include_directories: pwd_inc,
)

