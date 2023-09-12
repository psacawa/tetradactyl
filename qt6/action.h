// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QList>
#include <QMetaObject>
#include <QWidget>

#include <map>

// c++ forbids  forward reference to enum HintMode
#include "controller.h"

using std::map;

namespace Tetradactyl {

class AbstractUiAction {
public:
  virtual QList<QWidget *> getHintables(QWidget *root = nullptr) = 0;
  virtual void accept(QWidget *widget) = 0;
  virtual ~AbstractUiAction() {}

  static AbstractUiAction *getActionByHintMode(HintMode);
};

class ActivateAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  ActivateAction() {}
  QList<QWidget *> getHintables(QWidget *root = nullptr) override;
  void accept(QWidget *widget) override;
  virtual ~ActivateAction() {}
};

class EditAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  EditAction() {}
  QList<QWidget *> getHintables(QWidget *root = nullptr) override;
  virtual ~EditAction() {}
};

class FocusInputAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  FocusInputAction() {}
  QList<QWidget *> getHintables(QWidget *root = nullptr) override;
  void accept(QWidget *widget) override ;
  virtual ~FocusInputAction() {}
};

class YankAction : public AbstractUiAction {
public:
  static const QList<const QMetaObject *> acceptableMetaObjects;
  YankAction() {}
  QList<QWidget *> getHintables(QWidget *root = nullptr) override;
  virtual ~YankAction() {}
};

extern map<HintMode, AbstractUiAction *> actionRegistry;

} // namespace Tetradactyl
