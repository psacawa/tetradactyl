#include <cstdio>
#include <cstdlib>
#define _GNU_SOURCE

#include <dlfcn.h>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

using std::cout;
using std::runtime_error;
using std::string;

#define UI_LIB_SONAME "some-dynamic-qt6-app-ui"

string uiLibName = "lib" UI_LIB_SONAME ".so";
string dlopenPath = "${ORIGIN}/" + uiLibName;

void (*runUi)(int argc, char *argv[]);

char *getOrigin() {
  char *origin = new char[BUFSIZ];
  void *mainMap = dlopen(NULL, RTLD_NOLOAD | RTLD_LAZY);
  if (mainMap == NULL) {
    fprintf(stderr, "%s: %s\n", "dlopen", dlerror());
    exit(EXIT_FAILURE);
  }
  if (dlinfo(mainMap, RTLD_DI_ORIGIN, (void *)origin) < 0) {
    fprintf(stderr, "%s: %s\n", "dlinfo RTLD_DI_ORIGIN", dlerror());
    exit(EXIT_FAILURE);
  }
  return origin;
}

int main(int argc, char *argv[]) {
  void *uiLibrary = dlopen(dlopenPath.data(), RTLD_NOW);
  if (uiLibrary == nullptr)
    throw runtime_error("dlopen " UI_LIB_SONAME);

  void (*runUi)(int argc, char *argv[]);
  runUi = (void (*)(int, char **))dlsym(uiLibrary, "runUi");
  if (runUi == nullptr)
    throw runtime_error("dlsym " UI_LIB_SONAME);

  runUi(argc, argv);
  return 0;
}
