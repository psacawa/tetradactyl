// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QLabel>
#include <QString>
#include <qobject.h>

#include "controller.h"
#include "hint.h"
#include "overlay.h"

namespace Tetradactyl {

HintLabel::HintLabel(QString text, QWidget *w, Overlay *parent)
    : QLabel(text, parent), selected(false) {
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

// May need to do something here to control how HintLabel is rendered when
// QPaintEvent is sent to it directly (not via Overlay::paintEvent)
void HintLabel::paintEvent(QPaintEvent *event) { QLabel::paintEvent(event); }

} // namespace Tetradactyl
