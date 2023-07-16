#include <QObject>

#include <dlfcn.h>

#define _SET_ORIGINAL_FUNCTION(var, T)                                         \
  ({                                                                           \
    T var;                                                                     \
    (var) = GET_ORIGINAL_FUNCTION(var);                                        \
  })
#define SET_ORIGINAL_FUNCTION(T) _SET_ORIGINAL_FUNCTION(original_function, T)
#define GET_ORIGINAL_FUNCTION(var)                                             \
  (typeof((var)))dlsym(RTLD_NEXT, thisFunctionGetMangledName());

const char *getMangledName(void *addr);
const char *thisFunctionGetMangledName();
void print_tree(QObject *obj);
