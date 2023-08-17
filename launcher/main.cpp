#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QList>
#include <QMetaEnum>
#include <QProcess>
#include <Qt>

#include <memory>
#include <qprocess.h>
#include <unistd.h>

#include "launcher.h"
#include "utils.h"

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

  qInfo() << parser->positionalArguments();

  if (auto posArgs = parser->positionalArguments(); posArgs.length() > 0) {
    // launch given command, with dynamic probing if necessary

    QProcess child;
    QProcessEnvironment childEnv(QProcessEnvironment::InheritFromParent);
    // FIXME 16/08/20 psacawa: dynamic probing
    childEnv.insert("LD_PRELOAD", "libtetradactyl-qt6.so");
    child.setProcessEnvironment(childEnv);
    QString command = posArgs[0];
    posArgs.pop_front();
    // TODO 15/08/20 psacawa: instead of this, let the child be freestanding
    child.setProcessChannelMode(QProcess::ForwardedChannels);
    child.start(command, posArgs);
    child.waitForFinished(-1);

  } else {
    // no command supplied, run launcher
    Tetradactyl::Launcher launcher;
    launcher.show();
    app.exec();
  }
  return 0;
}
