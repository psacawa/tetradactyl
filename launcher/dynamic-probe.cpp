#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <pthread.h>

#include <string>

// NO QT ALLOWED!!

#define ARRAY_LEN(arr) sizeof(arr) / sizeof(arr[0])

using std::string;

using DlopenFunc = void *(*)(const char *, int);

// Map of widget lib names to tetradactyl backend libnames.Note that this gets
// used in a __attribute__((constructor)) context, so any C++ classes,
// specifically STL maps are forbidden here.
// TODO 16/08/20 psacawa: support more complex detection mechanism.
const char *backendMap[][2] = {{"libQt5Widgets", "libtetradactyl-qt5.so"},
                               {"libQt6Widgets", "libtetradactyl-qt6.so"},
                               {"libgtk-4", "libtetradactyl-gtk4.so"},
                               {"libgtk-3", "libtetradactyl-gtk3.so"}};

// use lambda as C-style callback to avoid global variable?
bool backendDetected = false;
const char *backend = NULL;

DlopenFunc originalDlopen = NULL;

extern "C" int dlIteratePhdrCallback(struct dl_phdr_info *info, size_t size,
                                     void *data) {
  for (int c = 0; c != ARRAY_LEN(backendMap); c++) {
    const char *backendName = backendMap[c][0];
    const char *tetradactylBackendName = backendMap[c][1];
    // if (string(info->dlpi_name).find(backendName) != string::npos) {
    if ((strstr(info->dlpi_name, backendName)) != NULL) {
      backendDetected = true;
      ssize_t *backendIdxPtr = (ssize_t *)data;
      *backendIdxPtr = c;
      return c;
    }
  }
  // no widget backend found loaded
  return 0;
};

// Though this is the dynamic probe, it's important to support detecting
// static dependencies as well. So check statically loaded shared objects with
// dl_iterate_phdr et al. and short-circuit the hooking if a match is found
bool checkLoadedDynLibs() {
  // backendIdx is a c-style return-parameter
  ssize_t backendIdx = -1;
  dl_iterate_phdr(dlIteratePhdrCallback, &backendIdx);
  if (backendIdx >= 0) {
    fprintf(stderr, "Target loaded %s. Initializing %s\n",
            backendMap[backendIdx][0], backendMap[backendIdx][1]);
    fflush(stderr);
    backendDetected = true;
    void *backendHandle = originalDlopen(backendMap[backendIdx][1], RTLD_NOW);
    if (backendHandle == NULL) {
      fprintf(stderr, "%s\n", dlerror());
      exit(1);
    }
    return true;
  }
  return false;
}

void *dlopen(const char *filename, int flags) {
  void *ret = originalDlopen(filename, flags);
  if (!backendDetected) {
    // check for a libname matching a backend and load the tetradactyldactyl
    // backend here. N.B. it's not enough to just check the `filename`
    // parameter. The lib may have transitive dependencies.
    fprintf(stderr, "Target dlopen'ed %s. Searching for dependency libs\n",
            filename);
    checkLoadedDynLibs();
  }
  return ret;
}

void installHooks() {}

void __attribute__((constructor)) initDynamicProbe() {
  fprintf(stderr, "Initializing Tetradactyl dynamic probe %p\n",
          (void *)pthread_self());
  originalDlopen = (DlopenFunc)((dlsym(RTLD_NEXT, "dlopen")));
  bool backendStaticallyDetected = checkLoadedDynLibs();
  if (backendStaticallyDetected) {
    fprintf(stderr, "Widget backend staticlly detected.");
  }
}
