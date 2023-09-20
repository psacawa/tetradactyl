// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <qslider.h>
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

  ui->applicationView->setModel(model);
  // ui->applicationView->hideColumn(0);

  connect(ui->launchButton, &QAbstractButton::clicked, this, [=] {
    auto selection = ui->applicationView->selectionModel()->selection();
    auto indexList = selection.indexes();
    if (indexList.size() > 0) {
      model->launch(indexList[0]);
    }
  });

  connect(ui->addButton, &QAbstractButton::clicked, [this] {
    QFileDialog fileDialog(this, tr("Select Program"), "/");
    fileDialog.setDirectory("/bin/");
    fileDialog.setFilter(QDir::AllEntries);
    if (!fileDialog.exec())
      return;

    auto selected = fileDialog.selectedFiles();
    qInfo() << "selected";
    qInfo() << selected;
    if (selected.length()) {
      // FIXME 15/08/20 psacawa: use Probe class
      int added = model->probeAndAddApp(QFileInfo(selected[0]));
      if (!added) {
        QMessageBox::warning(
            this, "Program not added",
            "An error occured and the application could not be added");
      }
    }
  });

  connect(ui->applicationView->selectionModel(),
          &QItemSelectionModel::selectionChanged,
          [](const QItemSelection &selected, const QItemSelection &deselected) {
            qInfo() << selected;
          });

  connect(ui->exitButton, &QAbstractButton::clicked, qApp, &QApplication::quit);
}

void Launcher::on_applicationView_activated(const QModelIndex &index) {
  qInfo() << __PRETTY_FUNCTION__;
  model->launch(index);
}
void Launcher::on_applicationView_doubleClicked(const QModelIndex &index) {
  qInfo() << __PRETTY_FUNCTION__;
}
void Launcher::on_launchButton_clicked(bool checked) {
  qInfo() << __PRETTY_FUNCTION__;
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
  ui->label->setVisible(false);
  ui->cancelButton->setEnabled(false);
  // ui->launchButton->setEnabled(false);
}

} // namespace Tetradactyl

#include "moc_launcher.cpp"
