// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <link.h>
#include <string>

#include <fmt/format.h>

#include "libnames.h"

#define MAX_ORIGIN_LEN 256
#define UNIMPLEMENTED(msg) fprintf(stderr, "Unimplemented %s\n", msg)

using fmt::format;
using std::cerr;
using std::cout;

// This is C++ but NO QT ALLOWED!!

#define ARRAY_LEN(arr) sizeof(arr) / sizeof(arr[0])

using std::string;

using DlopenFunc = void *(*)(const char *, int);

const char *backendMap[][2] = {{GTK3_LIB, GTK3_TETRADACTYL_LIB},
                               {GTK4_LIB, GTK4_TETRADACTYL_LIB},
                               {QT5_LIB, QT5_TETRADACTYL_LIB},
                               {QT6_LIB, QT6_TETRADACTYL_LIB}};

// use lambda as C-style callback to avoid global variable?
bool backendDetected = false;
const char *backend = NULL;

DlopenFunc originalDlopen = NULL;

void dlDie(const char *msg) {
  cerr << format("{}: {}", msg, dlerror());
  exit(EXIT_FAILURE);
}

// is extern "C" really necessary here?
extern "C" int dlIteratePhdrCallback(struct dl_phdr_info *info, size_t size,
                                     void *data) {
  for (int c = 0; c != ARRAY_LEN(backendMap); c++) {
    const char *backendName = backendMap[c][0];
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

bool stringHasDST(const string &inp, const string &dst) {
  return inp.find("${" + dst + "}") != string::npos ||
         inp.find("$" + dst) != string::npos;
}

void stringReplaceDST(string &inp, const string &dst,
                      const string &substitution) {
  auto replaceWhileFound = [&inp, substitution](const string &pattern) {
    while (1) {
      string::size_type pos = inp.find(pattern);
      if (pos == string::npos) {
        break;
      }
      inp.replace(pos, pattern.length(), substitution);
    }
  };
  replaceWhileFound("${" + dst + "}");
  replaceWhileFound(dst);
}

// UWAGA: the glibc uses an encapsulation-breaking technique to resolve the
// ${ORIGIN} DST in the filename argument. It uses __builtin_return_address(0)
// to discover the DSO to which the caller belongs. Then it resolves ${ORIGIN}
// according to the corresponding struct link_map and it's l_origin attribute.
// What this means is that if calls to dlopen are intercepted via e.g.
// LD_PRELOAD, then the wrong caller is recorded, and effectively these DSTs are
// resolved relative to libpreload.so. A naive implementation means  that
// LD_PRELOAD + DSTs like ORIGIN + intercepting cannot work at the same time.
// Therefore, we must *manually* resolve the DSTs in libpreload.so, and pass the
// expanded string to libc's dlopen.
bool resolveDSTs(string &dlopenPath) {
  bool changed = false;
  if (stringHasDST(dlopenPath, "ORIGIN")) {
    changed = true;
    // formatting lib {fmt} forces escaping {,}
    cerr << format("${{ORIGIN}} DST found in dlopen request for {}\n",
                   dlopenPath);
    void *mainMap = originalDlopen(NULL, RTLD_NOLOAD | RTLD_LAZY);
    if (mainMap == NULL)
      dlDie("dlopen");
    char buf[MAX_ORIGIN_LEN];
    if (dlinfo(mainMap, RTLD_DI_ORIGIN, buf) < 0)
      dlDie("dlinfo RTLD_DI_ORIGIN");
    stringReplaceDST(dlopenPath, "ORIGIN", buf);
  }
  if (stringHasDST(dlopenPath, "LIB")) {
    changed = true;
    cerr << format("${{LIB}} DST found in dlopen request for {}\n", dlopenPath);
    UNIMPLEMENTED("replace ${LIB} DST");
  }
  if (stringHasDST(dlopenPath, "PLATFORM")) {
    changed = true;
    cerr << format("${{PLATFORM}} DST found in dlopen request for {}\n",
                   dlopenPath);
    UNIMPLEMENTED("replace ${PLATFORM} DST");
  }
  return changed;
}

void *dlopen(const char *dlopenPathCstr, int flags) {
  string dlopenPath(dlopenPathCstr ? dlopenPathCstr : "");
  // resolve DSTs in dlopen path, see resolveDSTs comment
  resolveDSTs(dlopenPath);

  void *ret = originalDlopen(dlopenPath.c_str(), flags);
  if (!backendDetected) {
    // check for a libname matching a backend and load the tetradactyldactyl
    // backend here. N.B. it's not enough to just check the `filename`
    // parameter. The lib may have transitive dependencies.
    cerr << format("Target dlopen'ed \"{}\". Searching for dependency libs\n",
                   dlopenPath.c_str());
    checkLoadedDynLibs();
  }
  return ret;
}

void __attribute__((constructor)) initDynamicProbe() {
  fprintf(stderr, "Initializing Tetradactyl dynamic probe %p\n",
          (void *)pthread_self());
  originalDlopen = (DlopenFunc)((dlsym(RTLD_NEXT, "dlopen")));
  bool backendStaticallyDetected = checkLoadedDynLibs();
  if (backendStaticallyDetected) {
    fprintf(stderr, "Widget backend statically detected.\n");
  }
}
