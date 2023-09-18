// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QMetaEnum>
#include <QMetaObject>
#include <QString>

#include "common.h"
#include "hint.h"
#include "overlay.h"

namespace Tetradactyl {

bool isTetradactylMetaObject(const QMetaObject *mo) {
  return mo == &HintLabel::staticMetaObject || mo == &Overlay::staticMetaObject;
}

} // namespace Tetradactyl
