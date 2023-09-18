#include <QTest>

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
  void statusIndicatorTest();
  void noHintsTest();

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
  QCOMPARE(modeChangedSpy->count(), 0);
  QTest::keyClicks(win, "f");
  QCOMPARE(modeChangedSpy->count(), 1);
  QCOMPARE(modeChangedSpy->takeAt(0).at(0),
           WindowController::ControllerMode::Hint);
  QTest::keyClicks(win, "ss");
  QCOMPARE(modeChangedSpy->count(), 1);
  QCOMPARE(modeChangedSpy->takeAt(0).at(0),
           WindowController::ControllerMode::Normal);
}

void BasicTest::statusIndicatorTest() {
  const QLabel *statusIndicator = overlay->statusIndicator();
  // bottom right corner of status indicator corresponds to corner of window
  QCOMPARE(statusIndicator->pos() + statusIndicator->rect().bottomRight(),
           win->rect().bottomRight());
  QVERIFY(statusIndicator->isVisible());
  QCOMPARE(statusIndicator->text(), "Normal");
  QTest::keyClick(win, Qt::Key_F);
  QTRY_COMPARE(statusIndicator->text(), "Hint");
  QTest::keyClicks(win, "ss");
  QTRY_COMPARE(statusIndicator->text(), "Normal");
}

void BasicTest::noHintsTest() {
  // no menu items in the Calculator
  // TODO 18/09/20 psacawa: determine appropriate behaviour
  QTest::keyClick(win, Qt::Key_M);
  // QTRY_COMPARE(hintedSpy->count(), 1);
  // QCOMPARE(cancelledSpy->count(), 1);

  // no mode change on empty hint
  // QCOMPARE(modeChangedSpy->count(), 0);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::BasicTest);
#include "basic_test.moc"
