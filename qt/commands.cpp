// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QLoggingCategory>
#include <QMap>
#include <QMessageBox>
#include <QTimer>

#include "commands.h"
#include "controller.h"
#include "logging.h"

LOGGING_CATEGORY_COLOR("tetradactyl.commands", Qt::cyan);

namespace Tetradactyl {

bool reset(QList<QString> argv) {
  logInfo << "reset";
  tetradactyl->resetWindows();
  return true;
}

struct Command {
  QString argv0;
  QString description;
  CommandFunc func;
};

#define DEFINE_COMMAND(argv0, description, func)                               \
  { argv0, {argv0, description, func}, }

static QMap<QString, Command> commandRegistry = {
    DEFINE_COMMAND("reset", "reset Tetradactyl", reset)};

void runCommand(QList<QString> argv) {
  Q_ASSERT(argv.length() > 0);
  auto command = commandRegistry.find(argv.at(0));
  if (command != commandRegistry.end()) {
    QTimer::singleShot(10, [command, argv]() { command.value().func(argv); });
  } else {
    QString errorMsg = QString("Command \"%1\" not found").arg(argv[0]);
    QMessageBox *box =
        new QMessageBox(QMessageBox::Warning, "Command not found", errorMsg);
    box->setWindowModality(Qt::ApplicationModal);
    box->open();
  }
}

} // namespace Tetradactyl
