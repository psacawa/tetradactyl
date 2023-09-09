// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QList>
#include <QMetaObject>
#include <QWidget>

#include "controller.h"

#include <map>

using std::map;

namespace Tetradactyl {

class AbstractUiAction {
public:
  virtual QList<QWidget *> getHintables(QWidget *root = nullptr) = 0;
  virtual void accept(QWidget *widget) = 0;
  virtual ~AbstractUiAction() {}

  static AbstractUiAction *getActionByHintMode(HintMode);

private:
  static map<HintMode, AbstractUiAction *> actionRegistry;
};

class ActivateAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  ActivateAction();
  QList<QWidget *> getHintables(QWidget *root = nullptr) override;
  virtual ~ActivateAction() {}

private:
  /* data */
};
} // namespace Tetradactyl
