// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QObject>
#include <QTest>

#include <qt/controller.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/layouts/basiclayouts/basiclayouts
#include <basiclayouts/dialog.h>

namespace Tetradactyl {
class EditableTest : public QtBaseTest {
  Q_OBJECT
public:
private slots:
  void init();
  void cleanup();
  void basicInputModeTest();

private:
  Dialog *win;
};

;
void EditableTest::init() {
  win = new Dialog;
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
}

void EditableTest::cleanup() { delete win; }

void EditableTest::basicInputModeTest() {}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::EditableTest);
#include "editable_test.moc"
