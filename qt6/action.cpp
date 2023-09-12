// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QList>
#include <QTabWidget>
#include <QTextEdit>
#include <QWidget>
#include <qlist.h>
#include <qobject.h>

#include <algorithm>
#include <iterator>

#include "action.h"

using std::copy_if;

namespace Tetradactyl {

map<HintMode, AbstractUiAction *> actionRegistry = {
    {HintMode::Activatable, new ActivateAction}};

AbstractUiAction *AbstractUiAction::getActionByHintMode(HintMode mode) {
  for (auto [mode_iter, action] : actionRegistry) {
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
  // TODO 08/09/20 psacawa: finish this
  return {};
}

void ActivateAction::accept(QWidget *widget) {
  QAbstractButton *button = qobject_cast<QAbstractButton *>(widget);
  button->click();
}

const QList<const QMetaObject *> FocusInputAction::acceptableMetaObjects = {
    &QLineEdit::staticMetaObject, &QTextEdit::staticMetaObject};

QList<QWidget *> FocusInputAction::getHintables(QWidget *root) {
  QList<QWidget *> descendants = root->findChildren<QWidget *>();
  QList<QWidget *> ret;
  auto filterLambda = [](QWidget *widget) {
    const QMetaObject *candidateMO = widget->metaObject();
    if (!widget->isVisible() || !widget->isEnabled())
      return false;

    for (auto mo : acceptableMetaObjects) {
      if (candidateMO->inherits(mo)) {
        return true;
      }
    }
    return true;
  };
  copy_if(descendants.begin(), descendants.end(), std::back_inserter(ret),
          filterLambda);
  return ret;
}
void FocusInputAction::accept(QWidget *widget) { widget->setFocus(); }

} // namespace Tetradactyl
