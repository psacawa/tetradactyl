// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QTest>

#include <launcher/common.h>
#include <launcher/probe.h>

namespace Tetradactyl {

class LauncherProbeTest : public QObject {
  Q_OBJECT
private slots:
  void basicProbeElfTest_data();
  void basicProbeElfTest();
  void isExecutableTest();
};

void LauncherProbeTest::basicProbeElfTest_data() {
  QTest::addColumn<QString>("file");
  QTest::addColumn<Tetradactyl::WidgetBackend>("backend");

  QTest::newRow("null -> none") << "/bin/ls" << WidgetBackend::Unknown;
  QTest::newRow("wireshark -> qt5") << "/bin/wireshark" << WidgetBackend::Qt5;
  QTest::newRow("qtcreator -> qt6") << "/bin/qtcreator" << WidgetBackend::Qt6;
  QTest::newRow("glade -> gtk3") << "/bin/glade" << WidgetBackend::Gtk3;
  QTest::newRow("gnome-calculator -> gtk4")
      << "/bin/gnome-calculator" << WidgetBackend::Gtk4;
}

void LauncherProbeTest::basicProbeElfTest() {
  QFETCH(QString, file);
  QFETCH(Tetradactyl::WidgetBackend, backend);
  QFileInfo info(file);
  if (!info.exists())
    QSKIP(qPrintable(u"%1 not found"_qs.arg(file)));

  QCOMPARE(Tetradactyl::probeBackendFromElfFile(file), backend);
}

void LauncherProbeTest::isExecutableTest() {
  QFileInfo ls("/bin/ls");
  QVERIFY(isElfExecutable(ls));

  QFileInfo pip("/bin/pip");
  QVERIFY(!isElfExecutable(pip));
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::LauncherProbeTest)
#include "probe_test.moc"
