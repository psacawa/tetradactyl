// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QComboBox>
#include <QTest>

#include <qt/common.h>
#include <qt/controller.h>
#include <qt/hint.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/widgets/painting/painterpaths/
#include <painterpaths/window.h>

namespace Tetradactyl {

class ComboBoxTest : public QtBaseTest {
  Q_OBJECT

private slots:
  void init();
  void cleanup();
  void basicAcceptTest();
  void clickAwayTest();
  void enterOnComboBoxListTest();

private:
  Window *win;
  QComboBox *fillRuleComboBox, *penColorComboBox;
};

void ComboBoxTest::init() {
  win = new Window();
  fillRuleComboBox = win->findChildren<QComboBox *>().at(0);
  penColorComboBox = win->findChildren<QComboBox *>().at(3);
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
}

void ComboBoxTest::cleanup() {
  delete controller;
  delete win;
}

void ComboBoxTest::basicAcceptTest() {
  pressKeys("f");
  QList<HintLabel *> hints = windowController->activeOverlay()->hints();
  QCOMPARE(hints.length(), 4);
  // all hinted objects where QComboBox
  for (auto hint : hints)
    QCOMPARE(hint->target->metaObject(), &QComboBox::staticMetaObject);

  pressKeys("a");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(windowController->controllerMode(), Hint);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeAt(0).at(1)),
           fillRuleComboBox);
  QCOMPARE(hintingFinishedSpy->count(), 0);
  QVERIFY(isDescendantOf(qApp->focusWidget(), fillRuleComboBox));

  pressKeys("s");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(hintingFinishedSpy->count(), 1);
  QCOMPARE(fillRuleComboBox->currentText(), "Winding");
}

void ComboBoxTest::clickAwayTest() {
  pressKeys("f");
  pressKeys("s");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(modeChangedSpy->count(), 1);
  // For some stupid reason mouseClick on a widget doesn't really have the
  // correct effect in these popup scenarios
  QTest::mouseClick(win->windowHandle(), Qt::LeftButton);
  QCOMPARE(windowController->controllerMode(), Normal);
  QCOMPARE(modeChangedSpy->count(), 2);
}

void ComboBoxTest::enterOnComboBoxListTest() {
  pressKeys("f");

  pressKeys("f");
  // pen color combobox
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeAt(0).at(1)),
           penColorComboBox);
  QCOMPARE(hintingFinishedSpy->count(), 0);

  pressKey(Qt::Key_Return);
  QEXPECT_FAIL(
      "", "Key_Return in combbox doesn't work as hoped (wrong overlay)", Abort);
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(hintingFinishedSpy->count(), 1);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::ComboBoxTest);
#include "combobox_test.moc"
