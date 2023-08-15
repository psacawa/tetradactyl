#include <QApplication>
#include <Qt>

#include "launcher.h"

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  Tetradactyl::Launcher launcher;
  launcher.show();

  app.exec();

  return 0;
}
