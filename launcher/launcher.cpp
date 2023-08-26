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

#include <QtGlobal>

#include <exception>
#include <stdexcept>

#include "launcher.h"
#include "probe.h"
#include "tablemodel.h"

using std::domain_error;
using std::string;

namespace Tetradactyl {

QJsonObject App::toJson() const {
  QJsonObject serializedApp;
  serializedApp["file"] = file.absoluteFilePath();
  QMetaEnum backendEnum = QMetaEnum::fromType<WidgetBackend>();
  serializedApp["backend"] = backendEnum.valueToKey(backend);
  return serializedApp;
}

App App::fromJson(const QJsonObject &json) {
  App ret;
  if (const QJsonValue fileValue = json["file"]; fileValue.isString()) {
    QString file = fileValue.toString();
    QFileInfo fileInfo(file);
    if (!fileInfo.exists()) {
      throw domain_error("binary in application cache not found");
    }
    ret.file = fileInfo;
  }
  if (const QJsonValue backendValue = json["backend"];
      backendValue.isString()) {
    QMetaEnum backendEnum = QMetaEnum::fromType<WidgetBackend>();
    QString backendQString = backendValue.toString();
    bool valueOk;
    ret.backend = WidgetBackend(
        backendEnum.keyToValue(backendQString.toUtf8(), &valueOk));
    if (!valueOk) {
      throw domain_error(
          "Backend in application cache does not correspond not supported");
    }
  }
  return ret;
}

Launcher::Launcher() {
  setWindowTitle("Tetradactyl Launcher");

  createLayout();

  model = new ApplicationListModel();
  new ApplicationTableModel;

  view->setModel(model);

  connect(launchButton, &QAbstractButton::clicked, this, [=] {
    auto selection = view->selectionModel()->selection();
    auto indexList = selection.indexes();
    if (indexList.size() > 0) {
      model->launch(indexList[0]);
    }
  });

  connect(addButton, &QAbstractButton::clicked, [this] {
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

  connect(exitButton, &QAbstractButton::clicked, qApp, &QApplication::quit);
  connect(view, &QAbstractItemView::activated, model,
          &ApplicationListModel::launch);

  ProbeThread *probe = new ProbeThread();
  connect(probe, &ProbeThread::foundTetradctylApp, model,
          &ApplicationListModel::addTetradactylApp);

  connect(probe, &ProbeThread::finished, model,
          &ApplicationListModel::cacheTetradactylApps);

  probe->start();
}

void Launcher::createLayout() {
  central = new QWidget;
  setCentralWidget(central);
  layout = new QVBoxLayout(central);

  view = new QListView();
  bottom = new QWidget;

  layout->addWidget(view);
  layout->addWidget(bottom);

  bottomLayout = new QHBoxLayout(bottom);
  launchButton = new QPushButton(tr("&Launch"));
  addButton = new QPushButton(tr("&Add"));
  refreshButton = new QPushButton(tr("Re&fresh database"));
  exitButton = new QPushButton(tr("E&xit"));

  bottomLayout->addWidget(launchButton);
  bottomLayout->addWidget(addButton);
  bottomLayout->addWidget(refreshButton);
  bottomLayout->addWidget(exitButton);
}

ApplicationListModel::ApplicationListModel() {}

int ApplicationListModel::rowCount(const QModelIndex &parent) const {
  return apps.size();
}

QVariant ApplicationListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();

  App *app = apps[index.row()];
  return QVariant(app->file.fileName());
}

void ApplicationListModel::addTetradactylApp(QFileInfo file,
                                             WidgetBackend backend) {
  beginInsertRows(QModelIndex(), apps.size(), apps.size() + 1);
  App *app = new App{file.baseName(), file, backend};
  apps.push_back(app);
  endInsertRows();
}

void ApplicationListModel::cacheTetradactylApps() {
  qInfo() << "Caching found Tetradactyl Apps";
  QDir cachePath = qEnvironmentVariable(
      "XDG_CACHE_HOME",
      QString("%1/.cache/tetradactyl").arg(QDir::home().absolutePath()));
  if (!cachePath.exists()) {
    if (!cachePath.mkpath(".")) {
      qWarning()
          << QString("Could not create cache dir %1").arg(cachePath.path());
      return;
    }
  }
  QFile cacheFile(cachePath.filePath("apps.json"));
  if (!cacheFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate)) {
    qWarning()
        << QString("Could not create cache dir %1").arg(cachePath.path());
    return;
  }
  QJsonArray appsJson;
  for (auto app : apps) {
    appsJson.append(app->toJson());
  }
  cacheFile.write(QJsonDocument(appsJson).toJson());
  cacheFile.close();
}

void ApplicationListModel::launch(const QModelIndex &index) {
  App *app = apps[index.row()];
  qInfo() << QString("Launching %1").arg(app->file.absoluteFilePath());
  QProcessEnvironment childEnv(QProcessEnvironment::InheritFromParent);
  if (app->backend == WidgetBackend::Qt6) {
    childEnv.insert("LD_PRELOAD", "libtetradactyl-qt.so");
  }
  QProcess tetradactylProcess;
  tetradactylProcess.setProcessEnvironment(childEnv);
  tetradactylProcess.start(app->file.absoluteFilePath());
  tetradactylProcess.waitForFinished(-1);
  // TODO 15/08/20 psacawa: pass args
}

} // namespace Tetradactyl

#include "moc_launcher.cpp"
