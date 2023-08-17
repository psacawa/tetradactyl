#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QMetaEnum>
#include <QProcess>
#include <Qt>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <unistd.h>

#include "launcher.h"
#include "utils.h"

extern char **environ;

using std::unique_ptr;

QCommandLineParser *createCommandLineParser() {
  QCommandLineParser *parser = new QCommandLineParser;
  parser->setApplicationDescription("Tetradactyl launcher");
  parser->addHelpOption();
  parser->addVersionOption();

  QMetaEnum backendEnum = QMetaEnum::fromType<Tetradactyl::WidgetBackend>();
  QString backendValues = join(keysFromEnum(backendEnum), ",");
  QCommandLineOption backendOption(
      QList<QString>{"b", "backend"},
      QString("Force load executable with given backend: {%1}")
          .arg(backendValues),
      "backend");
  parser->addOption(backendOption);

  QCommandLineOption dynamicProbeOption(
      {"d", "dynamic"}, "Force dynamically probe executable for backend");
  parser->addOption(dynamicProbeOption);

  QCommandLineOption listOption(
      {"l", "list"},
      "List executables in database that Tetradactyl knows about");
  parser->addOption(listOption);

  parser->addPositionalArgument("executable", "Executable in PATH to launch");

  return parser;
}

// If the laucnher is launched without a concrete executable to run, it starts
// the UI, and tries to load the cache. If it doesn't exist, it automatically
// starts statically  probing executables on the file system.
// If the launcher is given an executable, it forgoes the launcher, and runs it.
// Unless --dynamic or --backend were given, it tries to find the executable in
// the application cache, to find the cached backend. Otherwise, it probes it
// dynamically.

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("tetradactyl");
  app.setApplicationVersion(QString::number(TETRADACTYL_VERSION));

  unique_ptr<QCommandLineParser> parser =
      unique_ptr<QCommandLineParser>(createCommandLineParser());
  parser->process(app);

  if (auto posArgs = parser->positionalArguments(); posArgs.length() > 0) {
    // launch given command, with dynamic probing if necessary
    // TODO 17/08/20 psacawa: implement for non-unix

    // TODO 17/08/20 psacawa: remove assumption on where to find libs
    // if (setenv("LD_PRELOAD",
    // "${ORIGIN}/../lib/libtetradactyl-dynamic-probe.so", 1) < 0) {
    QDir launcherOrigin = QFileInfo(getLocationOfThisProgram()).dir();
    QString preloadVar = QString("%1/../lib/libtetradactyl-dynamic-probe.so")
                             .arg(launcherOrigin.path());
    if (setenv("LD_PRELOAD", preloadVar.toLocal8Bit().data(), 1) < 0) {
      perror("setenv");
      exit(1);
    }
    execvpe(argv[1], &argv[1], environ);
  } else {
    // no command supplied, run launcher
    Tetradactyl::Launcher launcher;
    launcher.show();
    app.exec();
  }
  return 0;
}
