#include <QtTest>
#include <launcher/common.h>

class FunctionalLauncherTest : public QObject {
  Q_OBJECT
public slots:

  void test1() { QVERIFY(0); }
};

QTEST_MAIN(FunctionalLauncherTest);
#include "functionallauncher.moc"
