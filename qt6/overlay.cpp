// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QLoggingCategory>
#include <qassert.h>

#include <algorithm>

#include "hint.h"
#include "logging.h"
#include "overlay.h"

using std::find_if;

LOGGING_CATEGORY_COLOR("tetradactyl.overlay", Qt::yellow);

namespace Tetradactyl {

Overlay::Overlay(QWidget *target) : QWidget(target) {
  Q_ASSERT(target != nullptr);
}

void Overlay::addHint(QString &text, QWidget *target) {
  logInfo << "informacja";
  HintData data = {new HintLabel(text, target),
                   target->mapTo(parentWidget(), target->pos())};
  hints.append(data);
}

void Overlay::removeHint(HintLabel *hint) {
  Q_ASSERT(hint->parentWidget() == this);
  // int index = hints.indexOf(hint);
  const auto &hintData =
      find_if(hints.begin(), hints.end(),
              [hint](HintData &data) { return data.label == hint; });
  if (hintData != hints.end()) {
    int index = hintData - hints.begin();
    Q_ASSERT(index >= 0);
    hints.remove(index);
  } else {
    logWarning << hint << "not a hint controlled by" << this;
  }
}

} // namespace Tetradactyl
