// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QMetaEnum>
#include <QMetaObject>
#include <QModelIndex>
#include <QPoint>
#include <QWidget>

#include <type_traits>

#define UNUSED [[maybe_unused]]

namespace Tetradactyl {

bool isTetradactylObject(QObject *obj);
bool isTetradactylMetaObject(const QMetaObject *mo);

bool isDescendantOf(QObject *descendant, QObject *ancestor);
bool isTetradactylWindow(QWidget *w);
bool isTetradactylOverlayable(QWidget *w);
QWidget *getToplevelWidgetForWindow(QWindow *win);

// Is there an instance  of  C in ancentors of  obj
template <typename C> C findAncestor(QObject *obj) {
  using Obj = std::remove_pointer_t<C>;
  while (obj != nullptr) {
    if (obj->metaObject() == &Obj::staticMetaObject)
      return reinterpret_cast<C>(obj);
    obj = obj->parent();
  }
  return nullptr;
}

extern const char promptStylesheet[];

} // namespace Tetradactyl
