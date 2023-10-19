// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QString>

#include "abi.h"

namespace Tetradactyl {

Abi::Abi(WidgetBackend backend, int major, int minor, int patch,
         QString compiler)
    : p_backend(backend), p_major(major), p_minor(minor), p_patch(patch),
      p_compiler(compiler) {}

bool Abi::isCompatibleWith(const Abi &other) const {
  return p_major == other.p_major && p_minor == other.p_minor;
}

bool Abi::operator==(const Abi &other) {
  return p_major == other.p_major && p_minor == other.p_minor &&
         p_patch == other.p_patch && p_compiler == other.p_compiler;
}

} // namespace Tetradactyl
