// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

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
