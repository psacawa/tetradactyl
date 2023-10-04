// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QTest>

#include <qt/hint.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/widgets/widgets/calculator
#include <calculator/calculator.h>

namespace Tetradactyl {

class BasicTest : public QtBaseTest {
  Q_OBJECT
public:
private slots:
  void init();
  void cleanup();
  void signalEmissionTest();
  void cancelTest();
  void statusIndicatorTest();
  void noHintsTest();
  void manualAcceptCurrentHintTest();
  void acceptedHintHighlightTest();

private:
  Calculator *win;
};

void BasicTest::init() {
  win = new Calculator();
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
}

void BasicTest::cleanup() {
  delete controller;
  delete win;
}

void BasicTest::signalEmissionTest() {
  // beginning
  QCOMPARE(modeChangedSpy->count(), 0);
  QCOMPARE(hintedSpy->count(), 0);
  QCOMPARE(acceptedSpy->count(), 0);
  // hinting
  QTest::keyClicks(win, "f");
  QCOMPARE(hintedSpy->count(), 1);
  QCOMPARE(hintedSpy->takeAt(0).at(0), HintMode::Activatable);
  QTRY_COMPARE(modeChangedSpy->count(), 1);
  QCOMPARE(modeChangedSpy->takeAt(0).at(0), ControllerMode::Hint);
  QCOMPARE(acceptedSpy->count(), 0);
  QCOMPARE(hintingFinishedSpy->count(), 0);
  // accepting
  QTest::keyClicks(win, "ss");
  QCOMPARE(hintedSpy->count(), 0);
  QCOMPARE(hintingFinishedSpy->count(), 1);
  QVERIFY2(hintingFinishedSpy->takeAt(0).at(0).toBool(),
           "hintingFinished signal had accepted == true");
  QTRY_COMPARE(modeChangedSpy->count(), 1);
  QCOMPARE(modeChangedSpy->takeAt(0).at(0), ControllerMode::Normal);
  QCOMPARE(acceptedSpy->count(), 1);
  auto args = acceptedSpy->takeAt(0);
  QCOMPARE(args.at(0), HintMode::Activatable);
}

void BasicTest::cancelTest() {
  QTest::keyClicks(win, "f");
  QTest::keyClicks(win, "s");
  QCOMPARE(cancelledSpy->count(), 0);
  QTest::keyClick(win, Qt::Key_Escape);
  QCOMPARE(cancelledSpy->count(), 1);
  QCOMPARE(cancelledSpy->takeAt(0).at(0), HintMode::Activatable);
  QCOMPARE(hintingFinishedSpy->count(), 1);
  QVERIFY2(!hintingFinishedSpy->takeAt(0).at(0).toBool(),
           "hintingFinished signal had accepted == false");
}

void BasicTest::statusIndicatorTest() {
  const QLabel *statusIndicator = overlay->statusIndicator();
  // bottom right corner of status indicator corresponds to corner of window
  QCOMPARE(statusIndicator->pos() + statusIndicator->rect().bottomRight(),
           win->rect().bottomRight());
  QVERIFY(statusIndicator->isVisible());
  QCOMPARE(statusIndicator->text(), "Normal");
  QTest::keyClick(win, Qt::Key_F);
  QTRY_VERIFY(statusIndicator->text().startsWith("Hint"));
  QTest::keyClicks(win, "ss");
  QTRY_COMPARE(statusIndicator->text(), "Normal");
}

void BasicTest::noHintsTest() {
  // no menu items in the Calculator
  // TODO 18/09/20 psacawa: determine appropriate behaviour
  QTest::keyClick(win, Qt::Key_M);
  QCOMPARE(overlay->hints().length(), 0);
  QCOMPARE(windowController->controllerMode(), ControllerMode::Normal);
}

void BasicTest::manualAcceptCurrentHintTest() {
  QTest::keyClicks(win, "f");
  // first hinted widget is Backspace button with "AA"
  QWidget *backspaceButton = overlay->selectedWidget();
  QTest::keyClick(win, Qt::Key_Return);
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeFirst().at(1)),
           backspaceButton);
}

void BasicTest::acceptedHintHighlightTest() {
  QTest::keyClicks(win, "f");
  QTest::keyClicks(win, "aa");
  QCOMPARE(acceptedSpy->count(), 1);
  QTest::qSleep(100);
  QVERIFY2(win->findChildren<HintLabel *>().length() == 1,
           "Accepted hint trace still visible");
  HintLabel *acceptedHint = win->findChild<HintLabel *>();
  QVERIFY(acceptedHint->isSelected());
  QTest::qWait(500);
  QVERIFY2(win->findChildren<HintLabel *>().length() == 0,
           "Hint trace deleted");
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::BasicTest);
#include "basic_test.moc"
