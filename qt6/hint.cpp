// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QLabel>
#include <QString>

#include "controller.h"
#include "hint.h"

namespace Tetradactyl {

HintLabel::HintLabel(QString text, QWidget *w)
    : QLabel(text, w), selected(false) {
  setStyleSheet(Tetradactyl::Controller::stylesheet);
  target = w;
}

/*
 * QtWidgets' CSS implemenation is not dyamic like the browser layout engine is.
 * To get selector using a property to update, you have to reinstall the
 * stylesheet.
 */
void HintLabel::setSelected(bool _selected) {
  if (selected != _selected) {
    selected = _selected;
    setStyleSheet(Controller::stylesheet);
  }
}
} // namespace Tetradactyl
