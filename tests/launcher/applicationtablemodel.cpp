#include <Qt>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtTest>

#include <cstdlib>
#include <sys/ptrace.h>

#include "common.h"
#include <launcher/applicationtablemodel.h>

using Tetradactyl::ApplicationTableModel;
using Tetradactyl::WidgetBackend;

class ApplicationTableModelTest : public QObject {
  Q_OBJECT
public:
private slots:
  // void initTestCase() { ptrace(PTRACE_TRACEME, 0, 0, 0); }
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
    QCOMPARE(query.value(0).toString(), "tetradactyl_apps");
    query.exec("select * from tetradactyl_apps");
    query.first();
    QSqlRecord resultRecord = query.record();
    QCOMPARE_GE(resultRecord.indexOf("name"), -1);
    QCOMPARE_GE(resultRecord.indexOf("path"), -1);
    QCOMPARE_GE(resultRecord.indexOf("backend"), -1);

    QCOMPARE(query.lastError().type(), QSqlError::NoError);
  }

  void testAddTetradactylApp() {
    QFileInfo app("/usr/bin/ls");
    QVERIFY2(model->rowCount() == 0, "Starts empty");
    model->addTetradactylApp(app, WidgetBackend::Gtk4);
    QVERIFY2(model->rowCount() == 1,
             "ApplicationTableModel::addTetradactylApp adds element to model");
  }

private:
  QDir xdgDataHome;

  ApplicationTableModel *model;
};

QTEST_MAIN(ApplicationTableModelTest);
#include "applicationtablemodel.moc"
