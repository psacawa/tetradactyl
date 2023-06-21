#define _GNU_SOURCE

#include <dlfcn.h>
#include <link.h>
#include <string.h>

#include "gtk-utils.h"
#include "tetradactyl.h"

#define MACRO DEFINE_ORIG_LOC_PTR
#include "replaced-symbols"
#undef MACRO

typedef struct symbol_map_entry_t {
  char *symbol_name;
  void *ptr_to_ptr_to_symbol_loc;
  /* wskaźnik do położenia w libgtk-proxy gdzie przechowane jest zmienna orig_*
   * która sama jest wskaźnikiem do libgtk-4 */
} symbol_map_entry_t;

symbol_map_entry_t map[] = {
/* {"gtk_init", &orig_gtk_init}, */
#define MACRO SYMBOL_MAP_ENTRY
#include "replaced-symbols"
#undef MACRO
};

/* analyze the elf and find the static symbols on the symbol table */
static void init_static_symbols(const char *lib_path) {
  /* int lib_fd = open(lib_path, int oflag, ...); */
}

/* you can't call dlopen or do really any interaction  with  RTLD in DSO
 * constructors */
void post_load_proxy_init() {
  void *libgtk = dlopen("libgtk-4.so.1", RTLD_LAZY | RTLD_NOLOAD);
  if (libgtk == NULL) {
    tetradactyl_critical("libgtk-4 was not loaded at load time");
  }
  struct link_map *libgtk_lm;
  if (dlinfo(libgtk, RTLD_DI_LINKMAP, &libgtk_lm) < 0) {
    tetradactyl_critical("couldn't retrieve link map");
  }
  dlclose(libgtk);
  init_static_symbols(libgtk_lm->l_name);
}

void __attribute__((constructor)) init_gtk_proxy() {
  tetradactyl_info("initializing tetradactyl-gtk");
  /* setup proxy function pointers */
  for (int i = 0; i != ARRAY_LEN(map); ++i) {
    void *org_symbol_loc = dlsym(RTLD_NEXT, map[i].symbol_name);
    if (org_symbol_loc == NULL) {
      tetradactyl_error("symbol %s not loaded: %s\n", map[i].symbol_name,
                        dlerror());
      exit(EXIT_FAILURE);
    }
    *(void **)(map[i].ptr_to_ptr_to_symbol_loc) = org_symbol_loc; /* ? */
  }
  post_load_proxy_init();
  /* other early setup */
  memset(&tconfig, 0, sizeof(tetradactyl_config));
  /* config from json? */
  /* tconfig.hintchars = "asdfghjkl"; */
  tconfig.hintchars = "asd";
}
