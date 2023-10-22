// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QListView>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QtGlobal>
#include <QtTest>

#include <launcher/launcher.h>
#include <launcher/utils.h>

#include "common.h"

namespace Tetradactyl {

class LauncherE2ETest : public QObject {
  Q_OBJECT
public:
  Launcher *launcher = nullptr;

private slots:

  void init();
  void cleanup();
  void addAppTest();
  void launchTest();

private:
  QAbstractButton *addButton;
  QAbstractButton *launchButton;
  QAbstractButton *buildDatabaseButton;
  QListView *applicationView;

  QScopedPointer<QTemporaryDir> home;
};

void LauncherE2ETest::init() {
  home.reset(new QTemporaryDir);
  QVERIFY(home->isValid());
  qputenv("HOME", home->path().toLocal8Bit());

  launcher = new Launcher();

#define SET_ATTRIBUTE(type, objName)                                           \
  objName = launcher->findChild<type *>(#objName);                             \
  QVERIFY(objName != nullptr);

  // this is only necessary because uic code can't be found by test
  SET_ATTRIBUTE(QAbstractButton, addButton);
  SET_ATTRIBUTE(QAbstractButton, launchButton);
  SET_ATTRIBUTE(QAbstractButton, buildDatabaseButton);
  SET_ATTRIBUTE(QListView, applicationView);

  launcher->show();
  if (!QTest::qWaitForWindowActive(launcher))
    QFAIL("window didn't become active");
}

void LauncherE2ETest::cleanup() {
  delete launcher;
  // removes the temp dir
  home.reset();
}

void LauncherE2ETest::launchTest() {
  // TODO 22/10/20 psacawa: finish this
}

void LauncherE2ETest::addAppTest() {}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::LauncherE2ETest)
#include "launcher_e2e_test.moc"
