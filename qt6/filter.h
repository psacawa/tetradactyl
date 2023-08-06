#pragma once

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

namespace Tetradactyl {
class Controller;
}

class KeyboardEventFilter : public QObject {
  Q_OBJECT
public:
  KeyboardEventFilter(QObject *obj = nullptr,
                      Tetradactyl::Controller *controller = nullptr);

protected:
  bool eventFilter(QObject *obj, QEvent *ev) override;

private:
  QObject *owner = nullptr;
  Tetradactyl::Controller *controller = nullptr;
};
