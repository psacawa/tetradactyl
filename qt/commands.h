// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QList>
#include <QString>

#include <map>

namespace Tetradactyl {

using CommandFunc = bool (*)(QList<QString> argv);

void runCommand(QList<QString> argv);

} // namespace Tetradactyl
