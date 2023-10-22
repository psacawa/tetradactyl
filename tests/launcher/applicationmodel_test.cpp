// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QObject>
#include <QTemporaryDir>
#include <QtTest>

#include <launcher/applicationmodel.h>

namespace Tetradactyl {

class ApplicationModelTest : public QObject {
  Q_OBJECT
public:
private slots:
  void init();
  void cleanup();
  void filesystemTest();
  void saveAndLoadTest();
  // void launchTest();

private:
  QTemporaryDir *home;
  ApplicationModel *model;
};

void ApplicationModelTest::init() {
  home = new QTemporaryDir;
  QVERIFY(home->isValid());
  qputenv("HOME", home->path().toLocal8Bit());

  model = new ApplicationModel;
}

void ApplicationModelTest::cleanup() {
  delete model;
  delete home;
}

void ApplicationModelTest::filesystemTest() {
  // needed to trigger XDG_DATA_HOME + db creation
  model->saveDatabase();
  QDir dataDir(u"%1/%2"_qs.arg(home->path()).arg(".local/share/tetradactyl"));
  QVERIFY2(QDir::home().exists(), qPrintable(QDir::homePath()));
  QVERIFY2(dataDir.exists(), qPrintable(dataDir.path()));
  QVERIFY(dataDir.exists("apps.json"));
}

void ApplicationModelTest::saveAndLoadTest() {
  model->probeAndAddApp("/bin/ls");
  model->probeAndAddApp("/usr/bin/wireshark");
  model->probeAndAddApp("/usr/share/applications/cmake-gui.desktop");
  QCOMPARE(model->rowCount(), 3);
  model->saveDatabase();

  ApplicationModel model2;
  model2.tryLoadDatabase();
  QCOMPARE(model2.rowCount(), 3);

  QModelIndex idx = model2.index(0, 0);
  QCOMPARE(idx.data(Qt::DisplayRole), "ls");
  QCOMPARE(idx.data(BackendRole), Unknown);

  idx = model2.index(1, 0);
  QCOMPARE(idx.data(Qt::DisplayRole), "wireshark");
  QCOMPARE(idx.data(BackendRole), WidgetBackend::Qt5);

  idx = model2.index(2, 0);
  QCOMPARE(idx.data(Qt::DisplayRole), "CMake");
  QCOMPARE(idx.data(BackendRole), WidgetBackend::Qt5);
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::ApplicationModelTest)
#include "applicationmodel_test.moc"
