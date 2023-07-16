#include <dlfcn.h>
#include <stdio.h>

void __attribute__((constructor)) tetradactyl_init() {
  printf("%s\n", __PRETTY_FUNCTION__);
}
