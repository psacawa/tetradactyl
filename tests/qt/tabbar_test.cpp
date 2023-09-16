// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QTabBar>
#include <QtTest>

#include "common.h"
#include <qt/common.h>
#include <qt/controller.h>
#include <qt/overlay.h>
#include <qtestcase.h>
#include <qtestsupport_core.h>

#include "tabdialog/tabdialog.h"

// based on  "git://qtbase/examples/widgets/dialogs/tabdialog"

class TabBarTest : public QObject {
  Q_OBJECT
public:
private slots:
  void init();
  void cleanup();
  void basicSetupTest();

private:
  TabDialog *tabdialog;
};

void TabBarTest::init() {
  tabdialog = new TabDialog(".");
  waitForWindowActiveOrFail(tabdialog);
}
void TabBarTest::cleanup() { delete tabdialog; }

void TabBarTest::basicSetupTest() {
  // QTest::qSleep(10000);
  QList<QTabBar *> tabBars = qApp->findChildren<QTabBar *>();
  QList<QWidget *> widgets = qApp->allWidgets();
  qInfo() << &widgets;
  QCOMPARE(widgets.length(), 1);
}

QTEST_MAIN(TabBarTest);
#include "tabbar_test.moc"
