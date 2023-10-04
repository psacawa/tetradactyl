// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QMessageBox>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>

#include <qt/commandline.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/widgets/widgets/calculator
#include <calculator/calculator.h>

namespace Tetradactyl {

class PromptTest : public QtBaseTest {
  Q_OBJECT
private slots:
  void init();
  void cleanup();
  void openCloseTest();
  void geometryTest();
  void unknownCommandTest();
  void controllerResetTest();

private:
  Calculator *win;
  CommandLine *prompt;
  QSignalSpy *acceptedSpy, *openedSpy, *closedSpy;
};

void PromptTest::init() {
  win = new Calculator();
  QtBaseTest::init();
  prompt = windowController->mainOverlay()->commandLine();
  acceptedSpy = new QSignalSpy(prompt, &CommandLine::accepted);
  openedSpy = new QSignalSpy(prompt, &CommandLine::opened);
  closedSpy = new QSignalSpy(prompt, &CommandLine::closed);

  waitForWindowActiveOrFail(win);
}

void PromptTest::cleanup() {
  delete win;
  delete controller;
}

void PromptTest::openCloseTest() {
  QVERIFY(!prompt->isVisible());
  QCOMPARE(openedSpy->count(), 0);
  QCOMPARE(closedSpy->count(), 0);

  QTest::keyClicks(win, ":");
  QTRY_COMPARE(openedSpy->count(), 1);
  QCOMPARE(closedSpy->count(), 0);
  QVERIFY(prompt->isVisible());

  QTest::keyClicks(win, "asdf");
  QCOMPARE(openedSpy->count(), 1);
  QCOMPARE(closedSpy->count(), 0);
  QVERIFY(prompt->isVisible());

  QTest::keyClick(win, Qt::Key_Escape);
  QCOMPARE(openedSpy->count(), 1);
  QCOMPARE(closedSpy->count(), 1);
  QVERIFY(!prompt->isVisible());
}

void PromptTest::geometryTest() {
  QTest::keyClicks(win, ":");
  QVERIFY(prompt->isVisible());
  QVERIFY(prompt->width() == win->width());
}

void PromptTest::unknownCommandTest() {
  QTest::keyClicks(win, ":");
  QTest::keyClicks(prompt, "fakeCmd");
  QCOMPARE(acceptedSpy->count(), 0);

  QTest::keyClick(prompt, Qt::Key_Return);
  QTRY_COMPARE(acceptedSpy->count(), 1);
  QVERIFY(!prompt->isVisible());

  qInfo() << qApp->activeModalWidget();
  QMessageBox *box = qobject_cast<QMessageBox *>(qApp->activeModalWidget());
  QVERIFY2(box != nullptr, "Error popup appeared");
  QVERIFY(box->text().contains("not found"));
}

void PromptTest::controllerResetTest() {
  QSignalSpy *resetSpy = new QSignalSpy(tetradactyl, &Controller::reset);

  // we use these to test if object was recycled
  QPointer<Controller> controllerPtr = tetradactyl;
  QPointer<WindowController> windowControllerPtr = windowController;
  qWarning() << tetradactyl << windowController;

  QTest::keyClicks(win, ":");
  QTest::keyClicks(prompt, "reset");
  QTest::keyClick(prompt, Qt::Key_Return);
  QCOMPARE(acceptedSpy->count(), 1);

  QEXPECT_FAIL("", "reset broken", Abort);
  QCOMPARE(resetSpy->count(), 1);
  QVERIFY2(controllerPtr, "tetradactyl wasn't recycled");
  QVERIFY2(!windowControllerPtr, "Original WindowController was killed");
  WindowController *newWindowController =
      tetradactyl->findControllerForWidget(win);
  // QVERIFY(newWindowController != nullptr);
  qWarning() << tetradactyl << newWindowController;
  QTRY_VERIFY(tetradactyl->findControllerForWidget(win) != nullptr);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::PromptTest);
#include "prompt_test.moc"
