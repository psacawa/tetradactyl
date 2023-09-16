// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTabBar>
#include <QTabWidget>
#include <QtTest>
#include <qtestcase.h>
#include <qtestkeyboard.h>

#include "common.h"
#include <qt/common.h>
#include <qt/controller.h>
#include <qt/hint.h>
#include <qt/overlay.h>

// from "git://qtbase/examples/widgets/dialogs/tabdialog"
#include <tabdialog/tabdialog.h>

namespace Tetradactyl {

class TabBarTest : public QtBaseTest {
  Q_OBJECT
private slots:
  void init();
  void cleanup();
  void basicSetupTest();
  void hintTest();
  void tabDisabledTest();
  void tabInvisibleTest();

private:
  TabDialog *tabDialog;
  QTabWidget *tabWidget;
  QTabBar *tabBar;
  QStackedWidget *stackedWidget;
};

void TabBarTest::init() {
  tabDialog = new TabDialog(".");
  tabWidget = tabDialog->findChild<QTabWidget *>();
  tabBar = tabDialog->findChild<QTabBar *>();
  stackedWidget = tabDialog->findChild<QStackedWidget *>();
  QtBaseTest::init();
  waitForWindowActiveOrFail(tabDialog);
}
void TabBarTest::cleanup() {
  delete controller;
  delete tabDialog;
}

void TabBarTest::basicSetupTest() {
  QList<QTabBar *> tabBars =
      windowController->target()->findChildren<QTabBar *>();
  QCOMPARE(tabBars.length(), 1);
  QCOMPARE(tabBar, tabBars.at(0));
  QList<QTabWidget *> tabWidgets =
      windowController->target()->findChildren<QTabWidget *>();
  QCOMPARE(tabWidgets.length(), 1);
  QCOMPARE(tabWidget, tabWidgets.at(0));
  QVERIFY(stackedWidget != nullptr);
  QVERIFY(stackedWidget->currentIndex() == 0);
}

void TabBarTest::hintTest() {
  QTest::keyClick(tabDialog, Qt::Key_F);
  QList<HintLabel *> hints = overlay->hints();
  QList<HintLabel *> tabBarHints = overlay->findHintsByTarget<QWidget>();

  QTRY_COMPARE(overlay->findHintsByTarget<QTabBar>().length(), 3);

  QSignalSpy tabChangedSpy(tabBar, &QTabBar::currentChanged);
  QTest::keyClick(tabDialog, Qt::Key_S);
  QTRY_COMPARE(tabChangedSpy.count(), 1);
  QCOMPARE(tabBar->currentIndex(), 1);
}

void TabBarTest::tabDisabledTest() {
  tabBar->setTabEnabled(0, false);
  QTest::keyClick(tabDialog, Qt::Key_F);
  QTRY_COMPARE(overlay->findHintsByTarget<QTabBar>().length(), 2);
}

void TabBarTest::tabInvisibleTest() {
#define NUM_TABS_ADDED 3
  for (int i = 0; i != NUM_TABS_ADDED; ++i)
    tabBar->addTab(QString("Other tab %i").arg(i));
  tabDialog->resize(400, 300);
  QTest::keyClick(tabDialog, Qt::Key_F);
  QTRY_VERIFY2(overlay->findHintsByTarget<QTabBar>().length() !=
                   tabBar->count(),
               "Not all tabs are hint because some are inVisible");
  qInfo() << overlay->findHintsByTarget<QTabBar>().length();
  // scroll bar
}

} // namespace Tetradactyl

// QTEST_MAIN must be outside of the namespace, or at least the very least a
// symbol "main" must be emitted.
QTEST_MAIN(Tetradactyl::TabBarTest);
#include "tabbar_test.moc"
