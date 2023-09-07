#include <QAbstractButton>
#include <QGroupBox>
#include <QList>
#include <QTabWidget>
#include <QWidget>
#include <qobject.h>

#include "action.h"

namespace Tetradactyl {

AbstractUiAction *AbstractUiAction::getActionByHintMode(HintMode mode) {
  for (auto [mode_iter, action] : AbstractUiAction::actionRegistry) {
    if (mode_iter == mode)
      return action;
  }
  Q_UNREACHABLE();
}

const QList<const QMetaObject *> ActivateAction::acceptableMetaObjects = {
    &QAbstractButton::staticMetaObject};

QList<QWidget *> ActivateAction::getHintables(QWidget *root) {
  QList<QWidget *> widgets = root->findChildren<QWidget *>();
  QList<QWidget *> activatable;
  for (auto descendant : widgets) {
    if (descendant->isEnabled()) {
      if (QAbstractButton *button = qobject_cast<QAbstractButton *>(descendant);
          button != nullptr) {
        widgets.append(button);
      } else if (QGroupBox *groupBox = qobject_cast<QGroupBox *>(descendant);
                 groupBox != nullptr) {
        if (groupBox->isCheckable()) {
          widgets.append(groupBox);
        }
      } else if (QTabWidget *tab = qobject_cast<QTabWidget *>(descendant);
                 tab != nullptr) {
      }
    }
  }
}

} // namespace Tetradactyl
