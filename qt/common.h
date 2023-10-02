// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QMetaEnum>
#include <QMetaObject>
#include <QModelIndex>
#include <QPoint>
#include <QWidget>

#define UNUSED [[maybe_unused]]

namespace Tetradactyl {

bool isTetradactylMetaObject(const QMetaObject *mo);

// Polymorphically convert value  of enum registered with meta-object system to
// QString.
template <typename Enum> QString enumKeyToValue(Enum value) {
  QMetaEnum me = QMetaEnum::fromType<Enum>();
  return QString::fromLocal8Bit(me.valueToKey(value));
}

const char promptStylesheet[] =
    "* { background-color: #444; color: white; font-family: Monospace; "
    " padding: 1px; }";

} // namespace Tetradactyl
