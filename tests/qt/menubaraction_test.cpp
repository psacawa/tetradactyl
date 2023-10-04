// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QTest>

#include <QMenu>
#include <QMenuBar>

#include <qt/hint.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/widgets/mainwindows/mainwindow
#include <mainwindow/mainwindow.h>

namespace Tetradactyl {

class MenuBarActionTest : public QtBaseTest {
  Q_OBJECT
public:
private slots:
  void init();
  void cleanup();

  void basicTwoStepMenuBarActionAcceptedTest();
  void menuOverlayTest();
  void threeStepMenuBarActionAcceptedTest();
  void stepChangeMakesOverlaysInvisibleTest();
  void twoStepMenuBarActionCancelledTest();
  void overlaysAddedTest();
  void cancellationOnMenuBarTest();
  void tracerInMenuTest();

private:
  static MainWindow::CustomSizeHintMap map;
  MainWindow *win;
  QMenuBar *menuBar;
  QList<QMenu *> allMenus;
  QMenu *fileMenu;
  QMenu *toolbarMenu;
};

void MenuBarActionTest::init() {
  win = new MainWindow(map);
  win->resize(800, 600);
  menuBar = win->menuBar();
  allMenus = menuBar->findChildren<QMenu *>(
      Qt::FindChildOption::FindDirectChildrenOnly);
  fileMenu = allMenus.at(0);
  toolbarMenu = allMenus.at(2);
  QtBaseTest::init();

  waitForWindowActiveOrFail(win);
}

MainWindow::CustomSizeHintMap MenuBarActionTest::map;

void MenuBarActionTest::cleanup() {
  delete controller;
  delete win;
}

void MenuBarActionTest::overlaysAddedTest() {
  for (auto menu : allMenus) {
    Overlay *menuOverlay = windowController->findOverlayForWidget(menu);
    QTRY_VERIFY2(menuOverlay != nullptr && menuOverlay != overlay,
                 "QMenu has overlay which isn't the window's overlay");
    QTRY_VERIFY2(menuOverlay->statusIndicator() == nullptr,
                 "QMenu overlay has no status indicator label");
  }
}

void MenuBarActionTest::stepChangeMakesOverlaysInvisibleTest() {
  QTest::keyClicks(win, "m");
  Overlay *fileMenuOverlay =
      windowController->findOverlayForWidget(menuBar->findChild<QMenu *>());
  QVERIFY(fileMenuOverlay != nullptr);
  QCOMPARE_GT(overlay->visibleHints().length(), 0);
  QCOMPARE(fileMenuOverlay->hints().length(), 0);

  QTest::keyClicks(win, "a");
  QCOMPARE_GT(fileMenuOverlay->hints().length(), 0);
}

void MenuBarActionTest::basicTwoStepMenuBarActionAcceptedTest() {
  QTest::keyClicks(win, "m");
  QCOMPARE(acceptedSpy->count(), 0);
  QTRY_COMPARE(hintedSpy->count(), 1);
  QCOMPARE((hintedSpy->takeAt(0).at(0)), HintMode::Menuable);
  QCOMPARE(modeChangedSpy->count(), 1);
  QCOMPARE(modeChangedSpy->takeFirst().takeAt(0), ControllerMode::Hint);

  // file menu
  QTRY_VERIFY(!fileMenu->isVisible());
  QTest::keyClicks(win, "a");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeAt(0).at(1)), menuBar);
  QTRY_VERIFY(fileMenu->isVisible());

  // load layout
  QCOMPARE(hintingFinishedSpy->count(), 0);
  QTest::keyClicks(win, "d");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeAt(0).at(1)), fileMenu);
  QCOMPARE(hintingFinishedSpy->count(), 1);
  QTRY_VERIFY(!fileMenu->isVisible());
}

void MenuBarActionTest::twoStepMenuBarActionCancelledTest() {
  QTest::keyClicks(win, "m");
  QTest::keyClicks(win, "j");
  Overlay *aboutMenuOverlay = windowController->activeOverlay();
  QMenu *aboutMenu = qobject_cast<QMenu *>(aboutMenuOverlay->parentWidget());
  QCOMPARE(aboutMenu->title(), "About");
  QCOMPARE(cancelledSpy->count(), 0);
  QVERIFY(aboutMenu->isVisible());
  QCOMPARE(aboutMenuOverlay->visibleHints().length(), 2);
  QTest::keyClick(win, Qt::Key_Escape);
  QCOMPARE(cancelledSpy->count(), 1);
  QCOMPARE(aboutMenuOverlay->hints().length(), 0);
  QCOMPARE(hintingFinishedSpy->count(), 1);
  // POLICY: cancelling hinting in menus doesn't close them
  // QVERIFY(aboutMenu->isVisible());
}

void MenuBarActionTest::cancellationOnMenuBarTest() {
  QTest::keyClicks(win, "m");
  QCOMPARE(overlay->hints().length(), 5);
  QTest::keyClick(win, Qt::Key_Escape);
  QCOMPARE(overlay->hints().length(), 0);
  QCOMPARE(cancelledSpy->count(), 1);
}

void MenuBarActionTest::menuOverlayTest() {
  QTest::keyClicks(win, "m");
  QTest::keyClicks(win, "f");
  // Dock Widgets menu has 12 actions, with 1 disabled
  Overlay *dockWidgetsMenuOverlay = windowController->activeOverlay();
  QMenu *dockWidgetsMenu =
      qobject_cast<QMenu *>(dockWidgetsMenuOverlay->parentWidget());
  QCOMPARE(dockWidgetsMenu->title(), "&Dock Widgets");
  QCOMPARE(dockWidgetsMenuOverlay->hints().length(), 11);
  QCOMPARE(dockWidgetsMenuOverlay->visibleHints().length(), 11);

  // tab
  QCOMPARE(dockWidgetsMenuOverlay->selectedHint()->text(), "AA");
  QTest::keyClick(win, Qt::Key_Tab);
  QCOMPARE(dockWidgetsMenuOverlay->selectedHint()->text(), "AS");
  // filter to 4 hints
  QTest::keyClicks(win, "s");
  QCOMPARE(dockWidgetsMenuOverlay->visibleHints().length(), 4);
  QCOMPARE(dockWidgetsMenuOverlay->selectedHint()->text(), "SA");
  // accept current

  QTest::keyClick(win, Qt::Key_Return);
  QCOMPARE(acceptedSpy->count(), 2);
}

void MenuBarActionTest::threeStepMenuBarActionAcceptedTest() {
  QTest::keyClicks(win, "m");
  QTest::keyClicks(win, "f");
  // Dock Widgets menu
  Overlay *dockWidgetsMenuOverlay = windowController->activeOverlay();
  QMenu *dockWidgetsMenu =
      qobject_cast<QMenu *>(dockWidgetsMenuOverlay->parentWidget());
  QCOMPARE(dockWidgetsMenu->title(), "&Dock Widgets");
  // In "black" submenu, 19 enabled acitons
  QTest::keyClicks(win, "aj");
  Overlay *blackMenuOverlay = windowController->activeOverlay();
  QMenu *blackMenu = qobject_cast<QMenu *>(blackMenuOverlay->parentWidget());
  QCOMPARE(blackMenu->title(), "Black");
  QCOMPARE(acceptedSpy->count(), 2);
  QCOMPARE(dockWidgetsMenuOverlay->visibleHints().length(), 0);
  QCOMPARE(blackMenuOverlay->hints().length(), 19);

  // accept
  QCOMPARE(hintingFinishedSpy->count(), 0);
  QTest::keyClicks(win, "aa");
  QCOMPARE(acceptedSpy->count(), 3);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeLast().at(1)), blackMenu);
  QCOMPARE(hintingFinishedSpy->count(), 1);
}

void MenuBarActionTest::tracerInMenuTest() {
  QTest::keyClicks(win, "m");
  QTest::keyClicks(win, "a");
  QMenu *fileMenu =
      qobject_cast<QMenu *>(windowController->activeOverlay()->parentWidget());
  QCOMPARE(fileMenu->title(), "&File");
  QTest::keyClicks(win, "d");
  // File menu item accepted
  QTest::qWait(100);
  QList<HintLabel *> remainingHints = fileMenu->findChildren<HintLabel *>();
  QCOMPARE(remainingHints.length(), 1);
  HintLabel *acceptedHint = remainingHints.at(0);
  QVERIFY(acceptedHint->isSelected());
  QTest::qWait(500);
  QCOMPARE(fileMenu->findChildren<HintLabel *>().length(), 0);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::MenuBarActionTest);
#include "menubaraction_test.moc"
