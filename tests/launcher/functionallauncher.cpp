#include <QtTest>
#include <launcher/common.h>

class FunctionalLauncherTest : public QObject {
  Q_OBJECT
public slots:

  void test1() {
    Tetradactyl::WidgetBackend backend = Tetradactyl::WidgetBackend::Gtk3;
    QVERIFY(0);
  }
};

QTEST_MAIN(FunctionalLauncherTest);
#include "functionallauncher.moc"
