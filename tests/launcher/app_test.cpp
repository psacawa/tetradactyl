// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QTest>

#include <launcher/app.h>
#include <launcher/common.h>

using Tetradactyl::WidgetBackend;

namespace Tetradactyl {

class LauncherAppTest : public QObject {
  Q_OBJECT
private slots:
  void executableFileAppTest_data();
  void executableFileAppTest();
  void desktopAppTest_data();
  void desktopAppTest();
};

void LauncherAppTest::executableFileAppTest_data() {
  QTest::addColumn<QString>("filePath");
  QTest::addColumn<WidgetBackend>("backend");

  QTest::newRow("okular") << "/usr/bin/okular" << WidgetBackend::Qt5;
  QTest::newRow("ls") << "/usr/bin/ls" << WidgetBackend::Unknown;
}
void LauncherAppTest::executableFileAppTest() {
  QFETCH(QString, filePath);
  QFETCH(WidgetBackend, backend);

  QFileInfo info(filePath);
  if (!info.exists())
    QSKIP(qPrintable(u"executable file %1 not availabe"_qs.arg(filePath)));

  auto app = new ExecutableFileApp(filePath);
  QCOMPARE(app->backend(), backend);
  // more?
}

void LauncherAppTest::desktopAppTest_data() {
  QTest::addColumn<QString>("desktopFilePath");
  QTest::addColumn<QString>("desktopId");
  QTest::addColumn<QString>("absolutePath");
  QTest::addColumn<WidgetBackend>("backend");

  QTest::newRow("wireshark")
      << "/usr/share/applications/org.wireshark.Wireshark.desktop"
      << "org.wireshark.Wireshark.desktop"
      << "/usr/bin/wireshark" << WidgetBackend::Qt5;

  QTest::newRow("mpv") << "/usr/share/applications/mpv.desktop"
                       << "mpv.desktop"
                       << "/usr/bin/mpv" << WidgetBackend::Unknown;
}
void LauncherAppTest::desktopAppTest() {
  QFETCH(QString, desktopFilePath);
  QFETCH(QString, desktopId);
  QFETCH(QString, absolutePath);
  QFETCH(WidgetBackend, backend);

  QFileInfo info(desktopFilePath);
  if (!info.exists())
    QSKIP(qPrintable(u"desktop file %1 not availabe"_qs.arg(desktopFilePath)));

  auto app = XdgDesktopApp::fromDesktopFile(desktopFilePath);
  QCOMPARE(app->desktopId(), desktopId);
  QCOMPARE(app->absolutePath(), absolutePath);
  QCOMPARE(app->backend(), backend);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::LauncherAppTest)
#include "app_test.moc"
