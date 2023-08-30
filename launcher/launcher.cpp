#include <QAbstractListModel>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListView>
#include <QMainWindow>
#include <QMetaEnum>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <QVariant>
#include <Qt>

#include <QFileInfo>

#include <QtGlobal>

#include <exception>
#include <stdexcept>

#include "applicationtablemodel.h"
#include "launcher.h"
#include "probe.h"
#include "ui_launcherwindow.h"

using std::domain_error;
using std::string;

namespace Tetradactyl {

Launcher::Launcher() : ui(new Ui::LauncherWindow) {
  setWindowTitle("Tetradactyl Launcher");

  ui->setupUi(this);
  model = ApplicationTableModel::createApplicationTableModel();

  fixupUi();

  ui->appTableView->setModel(model);
  ui->appTableView->hideColumn(0);
  ui->appTableView->setSelectionBehavior(
      QAbstractItemView::SelectionBehavior::SelectRows);

  // model->addTetradactylApp(QFileInfo("/bin/anki"), WidgetBackend::Qt6);
  qInfo() << model->rowCount();

  connect(ui->launchButton, &QAbstractButton::clicked, this, [=] {
    auto selection = ui->appTableView->selectionModel()->selection();
    auto indexList = selection.indexes();
    if (indexList.size() > 0) {
      /*
       * model->launch(indexList[0]);
       */
    }
  });

  connect(ui->addButton, &QAbstractButton::clicked, [this] {
    QFileDialog fileDialog(this, tr("Select Program"), "/");
    fileDialog.setFilter(QDir::Executable | QDir::Dirs);
    if (!fileDialog.exec()) {
      // nothing selected from dialog
      return;
    }
    auto selected = fileDialog.selectedFiles();
    if (selected.length()) {
      // FIXME 15/08/20 psacawa: use Probe class
      model->addTetradactylApp(QFileInfo(selected[0]), WidgetBackend::Qt6);
    }
  });

  connect(ui->exitButton, &QAbstractButton::clicked, qApp, &QApplication::quit);
  /*
   *   connect(view, &QAbstractItemView::activated, model,
   *           &ApplicationTableModel::launch);
   *
   *   ProbeThread *probe = new ProbeThread();
   *   connect(probe, &ProbeThread::foundTetradctylApp, model,
   *           &ApplicationTableModel::addTetradactylApp);
   *
   *   connect(probe, &ProbeThread::finished, model,
   *           &ApplicationTableModel::cacheTetradactylApps);
   */

  // probe->start();
}

// last-mile modifications to UI initalization that either can't do, or it's too
// inconvenient
void Launcher::fixupUi() {
  ui->appTableView->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeMode::Stretch);

  // groupbox
  ui->label->setVisible(false);
  ui->cancelButton->setEnabled(false);
  ui->launchButton->setEnabled(false);
}

/*
 * QVariant ApplicationTableModel::data(const QModelIndex &index, int role)
 * const { if (!index.isValid()) return QVariant(); if (role != Qt::DisplayRole)
 *     return QVariant();
 *
 *   App *app = apps[index.row()];
 *   return QVariant(app->file.fileName());
 * }
 */

/*
 * void ApplicationTableModel::cacheTetradactylApps() {
 *   qInfo() << "Caching found Tetradactyl Apps";
 *   QDir cachePath = qEnvironmentVariable(
 *       "XDG_CACHE_HOME",
 *       QString("%1/.cache/tetradactyl").arg(QDir::home().absolutePath()));
 *   if (!cachePath.exists()) {
 *     if (!cachePath.mkpath(".")) {
 *       qWarning()
 *           << QString("Could not create cache dir %1").arg(cachePath.path());
 *       return;
 *     }
 *   }
 *   QFile cacheFile(cachePath.filePath("apps.json"));
 *   if (!cacheFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate)) {
 *     qWarning()
 *         << QString("Could not create cache dir %1").arg(cachePath.path());
 *     return;
 *   }
 *   QJsonArray appsJson;
 *   for (auto app : apps) {
 *     appsJson.append(app->toJson());
 *   }
 *   cacheFile.write(QJsonDocument(appsJson).toJson());
 *   cacheFile.close();
 * }
 */

} // namespace Tetradactyl

#include "moc_launcher.cpp"
