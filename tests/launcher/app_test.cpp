// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QTest>

#include <launcher/app.h>
#include <launcher/common.h>

using Tetradactyl::WidgetBackend;

class LauncherAppTest : public QObject {
  Q_OBJECT
private slots:
  void desktopAppTest_data();
  void desktopAppTest();
};

void LauncherAppTest::desktopAppTest_data() {}
void LauncherAppTest::desktopAppTest() {}

QTEST_MAIN(LauncherAppTest)
#include "app_test.moc"
