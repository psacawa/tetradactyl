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
  displayModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

  fixupUi();

  ui->applicationView->setModel(displayModel);

  connect(ui->launchButton, &QAbstractButton::clicked, this,
          &Launcher::onLaunchButtonClicked);

  connect(ui->buildDatabaseButton, &QPushButton::clicked, this, [this] {
    ui->spinner->start();
    sourceModel->initiateBuildAppDatabase();
  });

  connect(sourceModel, &ApplicationModel::appDatabaseBuilt, this,
          &Launcher::reportScanResults);
  connect(ui->applicationView, &QAbstractItemView::doubleClicked, this,
          &Launcher::onApplicationViewActivated);
  connect(ui->searchLineEdit, &QLineEdit::textChanged, this,
          &Launcher::onSearchTextChanged);
}

void Launcher::onApplicationViewActivated(const QModelIndex &index) {
  try {
    sourceModel->launch(index);
  } catch (exception &e) {
    QMessageBox::warning(this, "Error", "error launching app");
  }
}

void Launcher::onAddButtonClicked(bool checked) {
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
}
void Launcher::onLaunchButtonClicked(bool checked) {
  QModelIndex currentIndex =
      ui->applicationView->selectionModel()->currentIndex();
  onApplicationViewActivated(currentIndex);
}

void Launcher::onSearchTextChanged(const QString &text) {
  displayModel->setFilterFixedString(text);
}

void Launcher::reportScanResults(int numNewAppsAdded) {
  ui->spinner->stop();
  QMessageBox::information(this, "Scan finished",
                           QString("%1 new apps found").arg(numNewAppsAdded));
};

// last-mile modifications to UI initalization that either can't do, or it's too
// inconvenient
void Launcher::fixupUi() {
  ui->applicationView->setEditTriggers(QListView::NoEditTriggers);

  // groupbox
  ui->cancelButton->setEnabled(false);
}

} // namespace Tetradactyl

#include "moc_launcher.cpp"
