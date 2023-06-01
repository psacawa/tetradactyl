project('tetradactyl-gtk', 'c', version: '0.1')

gtk4_dep = dependency('gtk4')
pwd_inc = include_directories('.')

# libtetradactyl-gtk.so

tetradactyl_sources = files([
  'tetradactyl-gtk.c',
  'intercepted-symbols.c',
  'gtk-utils.c',
])

tetradactyl_headers =  files(['tetradactyl.h'])

shared_library('tetradactyl-gtk',
  sources: tetradactyl_sources,
  dependencies: [gtk4_dep],
  gnu_symbol_visibility: 'hidden',
  include_directories: pwd_inc,
)

# gtk-demo

demo_sources = files(['gtk-demo.c'])
executable('gtk-demo', 
  sources: demo_sources,
  dependencies: [gtk4_dep],
  include_directories: pwd_inc,
)
