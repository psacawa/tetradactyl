// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QDebug>
#include <QLoggingCategory>
#include <QMap>
#include <QtGlobal>

extern QMap<const char *, Qt::GlobalColor> lcColorMap;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

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
#else

// TODO 13/09/20 psacawa: figure out Qt5 logging
#define LOGGING_CATEGORY_COLOR(cat, color)
#define logDebug qDebug()
#define logInfo qInfo()
#define logWarning qWarning()
#define logCritical qCritical()
#endif

#define BLACK "\x1b\x5b\x33\x30\x6d"

#define GREEN_INTENSE "\x1b\x5b\x39\x32\x6d"
#define RED_INTENSE "\x1b\x5b\x39\x31\x6d"
#define BLUE_INTENSE "\x1b\x5b\x39\x34\x6d"
#define YELLOW_INTENSE "\x1b\x5b\x39\x33\x6d"
#define MAGENTA_INTENSE "\x1b\x5b\x39\x35\x6d"
#define CYAN_INTENSE "\x1b\x5b\x39\x36\x6d"

#define YELLOW_BG "\x1b\x5b\x34\x33\x6d"
#define ORANGE_BG "\x1b\x5b\x34\x31\x6d"
#define RED_BG "\x1b\x5b\x34\x31\x6d"

#define BOLD "\x1b\x5b\x31\x6d"
#define RESET_FOREGROUND "\x1b\x5b\x33\x39\x6d"
#define RESET_BACKGROUND "\x1b\x5b\x34\x39\x6d"
#define RESET RESET_BACKGROUND RESET_FOREGROUND

void colorMessageHandler(QtMsgType type, const QMessageLogContext &ctx,
                         const QString &msg);
