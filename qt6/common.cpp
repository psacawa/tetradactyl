#include <QObject>

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <iostream>

#include "common.h"

using std::cout;

const char *getMangledName(void *addr) {
  Dl_info dlinfo;
  if (dladdr(addr, &dlinfo) == 0) {
    fprintf(stderr, "dladdr: %s\n", dlerror());
    exit(1);
  }
  return dlinfo.dli_sname;
}

const char *thisFunctionGetMangledName() {
  return getMangledName(__builtin_return_address(0));
}

void print_tree_rec(QObject *obj, int depth) {
  for (int i = 0; i != depth; ++i)
    printf("\t");
  cout << obj->objectName().toStdString() << '\n';
  for (int i = 0; i != obj->children().length(); ++i) {
    QObject *child = obj->children().at(i);
    print_tree_rec(child, depth + 1);
  }
}
void print_tree(QObject *obj) { print_tree_rec(obj, 0); }
