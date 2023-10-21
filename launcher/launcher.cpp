// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <exception>
#include <stdexcept>

#include "applicationmodel.h"
#include "launcher.h"
#include "probe.h"
#include "ui_launcherwindow.h"
#include "waitingspinnerwidget.h"

using std::domain_error;
using std::exception;
using std::string;

namespace Tetradactyl {

Launcher::Launcher() : ui(new Ui::LauncherWindow) {
  setWindowTitle("Tetradactyl Launcher");

  ui->setupUi(this);
  sourceModel = new ApplicationModel(this);
  sourceModel->tryLoadDatabase();

  displayModel = new QSortFilterProxyModel(this);
  displayModel->setSourceModel(sourceModel);

  fixupUi();

  ui->applicationView->setModel(displayModel);
  // ui->applicationView->hideColumn(0);

  connect(ui->launchButton, &QAbstractButton::clicked, this, [=] {
    auto selection = ui->applicationView->selectionModel()->selection();
    auto indexList = selection.indexes();
    if (indexList.size() > 0) {
      sourceModel->launch(indexList[0]);
    }
  });

  connect(ui->addButton, &QAbstractButton::clicked, [this] {
    QFileDialog fileDialog(this, tr("Select Program"), "/");
    fileDialog.setDirectory("/usr/local/bin/");
    fileDialog.setFilter(QDir::AllEntries);
    if (!fileDialog.exec())
      return;

    auto selected = fileDialog.selectedFiles();
    if (selected.length()) {
      try {
        sourceModel->probeAndAddApp(selected[0]);
      } catch (exception &e) {
        QMessageBox::warning(this, "Program not added", e.what());
      }
    }
  });

  connect(ui->buildDatabaseButton, &QPushButton::clicked, this, [this] {
    qobject_cast<WaitingSpinnerWidget *>(ui->spinner)->start();
    sourceModel->initiateBuildAppDatabase();
  });

  connect(sourceModel, &ApplicationModel::appDatabaseBuilt, this,
          [this](int numNewAppsAdded) {
            qobject_cast<WaitingSpinnerWidget *>(ui->spinner)->stop();
            QMessageBox::information(
                this, "Scan finished",
                QString("%1 new apps found").arg(numNewAppsAdded));
          });

  connect(ui->applicationView, &QAbstractItemView::doubleClicked, sourceModel,
          [this](const QModelIndex &index) {
            try {
              sourceModel->launch(index);
            } catch (exception &e) {
              QMessageBox::warning(this, "Error", "error launching app");
            }
          });
}

void Launcher::on_applicationView_activated(const QModelIndex &index) {
  sourceModel->launch(index);
}
void Launcher::on_launchButton_clicked(bool checked) {
  QModelIndex currentIndex =
      ui->applicationView->selectionModel()->currentIndex();
  ui->applicationView->activated(currentIndex);
}

// last-mile modifications to UI initalization that either can't do, or it's too
// inconvenient
void Launcher::fixupUi() {
  // ui->applicationView->horizontalHeader()->setSectionResizeMode(
  //     QHeaderView::ResizeMode::Stretch);
  ui->applicationView->setEditTriggers(QListView::NoEditTriggers);

  // groupbox
  ui->cancelButton->setEnabled(false);
  // ui->launchButton->setEnabled(false);

  // spinner
  delete ui->spinner;
  ui->spinner = new WaitingSpinnerWidget(ui->spinnerContainer);
}

} // namespace Tetradactyl

#include "moc_launcher.cpp"
