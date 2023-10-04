// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QLineEdit>
#include <QWidget>

#include "commandline.h"
#include "common.h"
#include "controller.h"

namespace Tetradactyl {

CommandLine::CommandLine(QWidget *parent) : QLineEdit(parent), p_isOpen(false) {
  setStyleSheet(promptStylesheet);

  connect(this, &CommandLine::returnPressed, this, [this] {
    QString cmdline = text();
    setText("");
    tetradactyl->executeCommand(cmdline);
    emit accepted(cmdline);

    hide();
    emit closed();
  });

  hide();
}

void CommandLine::open() { setOpened(true); }

void CommandLine::setOpened(bool nowOpened) {
  bool changed = (nowOpened != p_isOpen);
  p_isOpen = nowOpened;
  if (nowOpened) {
    show();
    setFocus();
  } else {
    hide();
  }
  if (changed) {
    emit nowOpened ? opened() : closed();
  }
}

void CommandLine::focusOutEvent(QFocusEvent *ev) {
  setOpened(false);
  QLineEdit::focusOutEvent(ev);
}

void CommandLine::focusInEvent(QFocusEvent *ev) {
  setOpened(true);
  QLineEdit::focusInEvent(ev);
}

void CommandLine::reportError() {
  // TODO 02/10/20 psacawa: report error in prompt?
}

} // namespace Tetradactyl
