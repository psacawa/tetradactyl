#pragma once

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMetaObject>

#include <vector>

using std::vector;

namespace Tetradactyl {
class Controller;

class KeyboardEventFilter : public QObject {
  Q_OBJECT
public:
  KeyboardEventFilter(QObject *obj = nullptr,
                      Tetradactyl::Controller *controller = nullptr);

protected:
  bool eventFilter(QObject *obj, QEvent *ev) override;

private:
  static vector<const QMetaObject *> inputMetaObjects;
  bool inputWidgetFocussed();
  QObject *owner = nullptr;
  Tetradactyl::Controller *controller = nullptr;
};
} // namespace Tetradactyl
