// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtTest>
#include <qtestcase.h>

#include "common.h"
#include <launcher/applicationtablemodel.h>
#include <launcher/common.h>

using Tetradactyl::App;
using Tetradactyl::ApplicationTableModel;
using Tetradactyl::WidgetBackend;

class ApplicationTableModelTest : public QObject {
  Q_OBJECT
public:
  ApplicationTableModelTest() {
    QSKIP("Skipping launcher tests until moved off QSqlTableModel");
  }
private slots:
  void init() {
    xdgDataHome = tempTestDir();
    qInfo() << xdgDataHome.absolutePath();
    qputenv("XDG_DATA_HOME", xdgDataHome.absolutePath().toLocal8Bit());

    model = ApplicationTableModel::createApplicationTableModel();
  }
  void cleanup() {
    xdgDataHome.removeRecursively();
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    delete model;
  }

  void testCreatesAndInitializesDB() {
    QFile sqliteFile = xdgDataHome.filePath("tetradactyl/data.sqlite");
    QVERIFY2(sqliteFile.exists(),
             "ApplicationTableModel instantiates sqlite sqliteFile");
    QCOMPARE(QSqlDatabase::connectionNames().length(), 1);

    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    QSqlQuery query(db);
    QVERIFY(query.exec("select name from sqlite_master"));
    query.first();
    QVERIFY2(query.value(0).toString() == "tetradactyl_apps",
             "tetradactyl_apps table was created");
    query.exec("select * from tetradactyl_apps");
    query.first();
    QSqlRecord resultRecord = query.record();
    QVERIFY2(resultRecord.indexOf("name") >= 0,
             "tetradactyl_apps table has a 'name' field");
    QVERIFY2(resultRecord.indexOf("path") >= 0,
             "tetradactyl_apps table has a 'path' field");
    QVERIFY2(resultRecord.indexOf("backend") >= 0,
             "tetradactyl_apps table has a 'backend' field");

    QVERIFY2(query.lastError().type() == QSqlError::NoError,
             "No DB errors during application initialization");
  }

  void testAddAndDeleteTetradactylApp() {
    QFileInfo app("/usr/bin/ls");
    QVERIFY2(model->rowCount() == 0, "Starts empty");
    model->addTetradactylApp(app, WidgetBackend::Gtk4);
    QVERIFY2(model->rowCount() == 1,
             "ApplicationTableModel::addTetradactylApp adds element to model");
    // TODO 01/09/20 psacawa: unfuck this all
    App *ls = model->findByName("ls");
    QVERIFY2(ls != nullptr, "added app found in ApplicationTableModel");
    QVERIFY2(model->rowCount() == 0,
             "ApplicationTableModel::remoteTetradactylApp removes element from "
             "model");
  }

private:
  QDir xdgDataHome;

  ApplicationTableModel *model;
};

QTEST_MAIN(ApplicationTableModelTest);
#include "applicationtablemodel.moc"
