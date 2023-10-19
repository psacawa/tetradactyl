// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QString>

#include "common.h"

namespace Tetradactyl {
class Abi {
public:
  Abi(WidgetBackend backend, int major = 0, int minor = 0, int patch = 0,
      QString compiler = QString());
  virtual ~Abi() {}

  int majorVersion();
  int minorVersion();
  int patchVersion();
  QString compilerVersion();
  bool isCompatibleWith(const Abi &other) const;
  bool operator==(const Abi &other);

private:
  int p_major;
  int p_minor;
  int p_patch;
  QString p_compiler;
  WidgetBackend p_backend;
};

inline int Abi::majorVersion() { return p_major; }
inline int Abi::minorVersion() { return p_minor; }
inline int Abi::patchVersion() { return p_patch; }
inline QString Abi::compilerVersion() { return p_compiler; }
} // namespace Tetradactyl
