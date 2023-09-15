#include <QDebug>
#include <QTabBar>
#include <QtTest>
#include <cstdlib>
#include <qtestcase.h>

class TabBarTest : public QObject {
  Q_OBJECT
public:
private slots:
  void test1() {
    QVERIFY2(true, "vacuous");
  }
};

QTEST_MAIN(TabBarTest);
#include "tabbar_test.moc"
