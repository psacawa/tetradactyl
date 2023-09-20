// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QTest>

#include <QMenu>
#include <QMenuBar>

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

  void basicTwoStepMenuBarActionTest();
  void overlaysAddedTest();
  void cancellationOnMenuBarTest();
  void cancellationOnQMenuTest();

private:
  static MainWindow::CustomSizeHintMap map;
  MainWindow *win;
  QMenuBar *menuBar;
  QList<QMenu *> menus;
  QMenu *fileMenu;
  QMenu *toolbarMenu;
};

void MenuBarActionTest::init() {
  win = new MainWindow(map);
  win->resize(800, 600);
  menuBar = win->menuBar();
  menus = menuBar->findChildren<QMenu *>(
      Qt::FindChildOption::FindDirectChildrenOnly);
  fileMenu = menus.at(0);
  toolbarMenu = menus.at(2);
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
}

MainWindow::CustomSizeHintMap MenuBarActionTest::map;

void MenuBarActionTest::cleanup() {
  delete controller;
  delete win;
}

void MenuBarActionTest::overlaysAddedTest() {
  for (auto menu : menus) {
    Overlay *menuOverlay = windowController->findOverlayForWidget(menu);
    QTRY_VERIFY2(menuOverlay != nullptr && menuOverlay != overlay,
                 "QMenu has overlay which isn't the window's overlay");
    QTRY_VERIFY2(menuOverlay->statusIndicator() == nullptr,
                 "QMenu overlay has no status indicator label");
  }
}

void MenuBarActionTest::basicTwoStepMenuBarActionTest() {
  QTest::keyClicks(win, "m");
  QCOMPARE(hintedSpy->count(), 1);
  QCOMPARE((hintedSpy->takeAt(0).at(0)), HintMode::Menuable);

  // file menu
  QTRY_VERIFY(!fileMenu->isVisible());
  QTest::keyClicks(win, "a");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeAt(0).at(0)), fileMenu);
  QTRY_VERIFY(fileMenu->isVisible());

  // load layout
  QTest::keyClicks(win, "s");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(qvariant_cast<QObject *>(acceptedSpy->takeAt(0).at(0)),
           fileMenu->actions().at(0));
  QTRY_VERIFY(!fileMenu->isVisible());
}

void MenuBarActionTest::cancellationOnMenuBarTest() {}

void MenuBarActionTest::cancellationOnQMenuTest() {}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::MenuBarActionTest);
#include "menubaraction_test.moc"
