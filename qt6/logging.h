// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QMap>

extern QMap<const char *, Qt::GlobalColor> lcColorMap;

#define LOGGING_CATEGORY_COLOR(cat, color)                                     \
  static const QLoggingCategory &lcThis() {                                    \
    static const QLoggingCategory category(cat);                               \
    lcColorMap.insert(cat, color);                                             \
    return category;                                                           \
  }

#define logDebug QT_MESSAGE_LOGGER_COMMON(lcThis, QtDebugMsg).debug()
#define logInfo QT_MESSAGE_LOGGER_COMMON(lcThis, QtInfoMsg).info()
#define logWarning QT_MESSAGE_LOGGER_COMMON(lcThis, QtWarningMsg).warning()
#define logCritical QT_MESSAGE_LOGGER_COMMON(lcThis, QtCriticalMsg).critical()

#define GREEN_INTENSE "\x1b\x5b\x39\x32\x6d"
#define RED_INTENSE "\x1b\x5b\x39\x31\x6d"
#define BLUE_INTENSE "\x1b\x5b\x39\x34\x6d"
#define YELLOW_INTENSE "\x1b\x5b\x39\x33\x6d"
#define MAGENTA_INTENSE "\x1b\x5b\x39\x35\x6d\x0a"
#define CYAN_INTENSE "\x1b\x5b\x39\x36\x6d\x0a"
#define BOLD "\x1b\x5b\x31\x6d\x0a"
#define RESET_FOREGROUND "\x1b\x5b\x33\x39\x6d"

void colorMessageHandler(QtMsgType type, const QMessageLogContext &ctx,
                         const QString &msg);
