// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QWidget>
#include <QtTest>

#include <qt/controller.h>

#include "common.h"

namespace Tetradactyl {

void waitForWindowActiveOrFail(QWidget *win) {
  win->show();
  if (!QTest::qWaitForWindowActive(win))
    QFAIL("window didn't become active");
}

} // namespace Tetradactyl
