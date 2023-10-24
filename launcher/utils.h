// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QDir>
#include <QList>
#include <QMetaEnum>
#include <QString>

void system_die(const char *msg);

#define DIE_IF_NEG(cmd)                                                        \
  ({                                                                           \
    typeof(cmd) ret = cmd;                                                     \
    if (ret < 0)                                                               \
      system_die(#cmd);                                                        \
    ret;                                                                       \
  })

QList<QString> keysFromEnum(QMetaEnum &e_num);
QString join(QList<QString> arr, QString sep);
QString getLocationOfThisProgram();
void mkdirRec(QDir dir, QString relPath);

// Polymorphically convert value of enum registered with meta-object system to
// QString and vice versa
template <typename Enum> QString enumValueToKey(Enum value) {
  QMetaEnum me = QMetaEnum::fromType<Enum>();
  return QString::fromLocal8Bit(me.valueToKey(value));
}
template <typename Enum> Enum enumKeyToValue(QString s) {
  QMetaEnum me = QMetaEnum::fromType<Enum>();
  return static_cast<Enum>(me.keyToValue(qPrintable(s)));
}
