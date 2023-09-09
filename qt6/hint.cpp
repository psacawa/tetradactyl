// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QLabel>
#include <QString>

#include "controller.h"
#include "hint.h"

namespace Tetradactyl {

HintLabel::HintLabel(QString text, QWidget *target) : QLabel(text, target) {
  setStyleSheet(Controller::stylesheet);
}
} // namespace Tetradactyl
