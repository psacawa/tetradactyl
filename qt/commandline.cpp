// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QLineEdit>
#include <QWidget>

#include "commandline.h"
#include "common.h"
#include "controller.h"

namespace Tetradactyl {

CommandLine::CommandLine(QWidget *parent) : QLineEdit(parent) {
  setStyleSheet(promptStylesheet);

  connect(this, &CommandLine::returnPressed, this, [this] {
    QString cmd = text();
    setText("");
    bool executed = tetradactyl->executeCommand(cmd);
    if (!executed) {
    }
    hide();
  });

  hide();
}

void CommandLine::open() {
  show();
  setFocus();
}

void CommandLine::focusOutEvent(QFocusEvent *ev) {
  hide();
  QLineEdit::focusOutEvent(ev);
}
void CommandLine::focusInEvent(QFocusEvent *ev) {
  show();
  QLineEdit::focusInEvent(ev);
}

void CommandLine::reportError() {
  // TODO 02/10/20 psacawa: report error in prompt
}

} // namespace Tetradactyl
