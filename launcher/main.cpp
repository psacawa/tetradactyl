#include <QCommandLineParser>
#include <QDir>
#include <unistd.h>

#include "common.h"
#include "launcher.h"
#include "utils.h"

extern char **environ;

using Tetradactyl::WidgetBackend;

// If the laucnher is launched without a concrete executable to run, it
// starts the UI, and tries to load the cache. If it doesn't exist, it
// automatically starts statically  probing executables on the file system.
// If the launcher is given an executable, it forgoes the launcher, and runs
// it. Unless --dynamic or --backend were given, it tries to find the
// executable in the application cache, to find the cached backend.
// Otherwise, it probes it dynamically.

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("tetradactyl");
  app.setApplicationVersion(QString::number(TETRADACTYL_VERSION));

  // create parser

  QCommandLineParser parser;
  parser.setApplicationDescription("Tetradactyl launcher");
  parser.addHelpOption();
  parser.addVersionOption();

  QMetaEnum backendEnum = QMetaEnum::fromType<Tetradactyl::WidgetBackend>();
  QString backendValues = join(keysFromEnum(backendEnum), ",");
  QCommandLineOption backendOption(
      QList<QString>{"b", "backend"},
      QString("Force load executable with given backend: {%1}")
          .arg(backendValues),
      "backend");
  parser.addOption(backendOption);

  QCommandLineOption dynamicProbeOption(
      {"d", "dynamic"}, "Force dynamically probe executable for backend");
  parser.addOption(dynamicProbeOption);

  QCommandLineOption listOption(
      {"l", "list"},
      "List executables in database that Tetradactyl knows about");
  parser.addOption(listOption);
  parser.addPositionalArgument("executable", "Executable in PATH to launch");

  // run tetradactyl
  parser.process(app);

  auto posArgs = parser.positionalArguments();
  if (parser.isSet(listOption)) {
    qWarning() << "list apps unimplemented";
    return 0;
  }
  if (posArgs.length() >= 1) {
    // launch given command, with dynamic probing if necessary
    // TODO 17/08/20 psacawa: implement for non-unix

    // TODO 17/08/20 psacawa: remove assumption on where to find libs
    // TODO 17/08/20 psacawa: remove this damn kruft

    // IF --backend or --dynamic-probe were passed, inject accordingly. If both,
    // fail with error message. If neither, check if app is in repository. If
    // so, inject accordingly. Else, dynamically probe.
    QString preloadedLib;
    WidgetBackend backend;
    if (parser.isSet(dynamicProbeOption) && parser.isSet(backendOption)) {
      qInfo() << "asdf";

      qCritical()
          << "--dynamic-probe and --backend are mutually exclusive options";
      return 1;
    }
    if (parser.isSet(backendOption)) {
      QString backendString = parser.value(backendOption);
      QMetaEnum me = QMetaEnum::fromType<WidgetBackend>();
      int backendInt = me.keyToValue(backendString.toStdString().c_str());
      if (backendInt < 0) {
        qCritical() << "Unrecognized backend" << backendString;
        return 1;
      }
      backend = static_cast<WidgetBackend>(backendInt);
      if (backend != WidgetBackend::Unknown) {
        preloadedLib = QString::fromStdString(backends[backend].tetradactylLib);
      }
    }
    if (preloadedLib == "") {
      preloadedLib = DYNAMIC_TETRADACTYL_LIB;
    }
    if (backend != WidgetBackend::Unknown) {
      QDir launcherOrigin = QFileInfo(getLocationOfThisProgram()).dir();
      QString preloadVar =
          QString("%1/../lib/%2").arg(launcherOrigin.path()).arg(preloadedLib);
      if (setenv("LD_PRELOAD", preloadVar.toLocal8Bit().data(), 1) < 0) {
        perror("setenv");
        exit(1);
      }
    }

    QString clientProgram = posArgs.first();
    const char *childArgv0 = clientProgram.toLocal8Bit().data();
    char *childArgv[posArgs.length() + 1];
    for (int i = 0; i != posArgs.length(); ++i) {
      childArgv[i] = posArgs[i].toLocal8Bit().data();
      printf("arg %d: %s\n", i, childArgv[i]);
    }
    childArgv[posArgs.length()] = NULL;
    execvpe(childArgv0, childArgv, environ);
  } else {
    // no command supplied, run launcher
    Tetradactyl::Launcher launcher;
    launcher.show();
    app.exec();
  }

  return 0;
}
