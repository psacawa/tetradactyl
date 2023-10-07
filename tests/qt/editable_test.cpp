// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QLineEdit>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QTest>
#include <QTextEdit>

#include <qt/controller.h>
#include <qt/hint.h>
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
  QTextEdit *lowerEditor;
  QTextEdit *upperEditor;
};

void EditableTest::init() {
  win = new Dialog;
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
  upperEditor = win->findChildren<QTextEdit *>().at(0);
  lowerEditor = win->findChildren<QTextEdit *>().at(1);
}

void EditableTest::cleanup() {
  delete win;
  delete tetradactyl;
}

void EditableTest::basicInputModeTest() {
  QList<const QMetaObject *> mos = {&QLineEdit::staticMetaObject,
                                    &QTextEdit::staticMetaObject};
  QTest::keyClicks(win, "gi");
  for (auto hint : windowController->mainOverlay()->hints()) {
    QVERIFY2(mos.contains(hint->target->metaObject()),
             "hints are either QLineEdit or QTextEdit");
  }

  QTest::keyClicks(win, "f");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(hintingFinishedSpy->count(), 1);
  QCOMPARE(win->focusWidget(), upperEditor);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::EditableTest);
#include "editable_test.moc"
