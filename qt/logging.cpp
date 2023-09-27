// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QtGlobal>

#include "logging.h"

extern "C" {
#if DEBUG && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// For printing in gdb
const char *qObject2Cstr(void *qobj) {
  return qPrintable(QDebug::toString((QObject *)qobj));
}
#endif
}

QMap<const char *, Qt::GlobalColor> lcColorMap = {};

static QMap<Qt::GlobalColor, QByteArray> ansiTermColorMap = {
    {Qt::red, RED_INTENSE},   {Qt::green, GREEN_INTENSE},
    {Qt::blue, BLUE_INTENSE}, {Qt::yellow, YELLOW_INTENSE},
    {Qt::cyan, CYAN_INTENSE}, {Qt::magenta, MAGENTA_INTENSE},
};

void colorMessageHandler(QtMsgType severity, const QMessageLogContext &ctx,
                         const QString &msg) {

  QByteArray prefix, suffix, categoryStr = ctx.category, severityStr;
#ifndef TETRADACTYL_QT_TEST
  Qt::GlobalColor color =
      lcColorMap.value(ctx.category, Qt::GlobalColor::color0);
  if (color != Qt::GlobalColor::color0) {
    categoryStr = ansiTermColorMap.value(color) + categoryStr +
                  QByteArrayLiteral(RESET_FOREGROUND);
  } else {
    if (strcmp(ctx.category, "default")) {
      qWarning() << "color not found " << ctx.category;
    }
  }
  switch (severity) {
  case QtWarningMsg:
    severityStr = "(" BLACK YELLOW_BG "WARNING" RESET ")";
    break;
  case QtCriticalMsg:
    severityStr = "(" RED_BG "CRITICAL" RESET_BACKGROUND ")";
    break;
  case QtFatalMsg:
    severityStr = "(" RED_BG "FATAL" RESET_BACKGROUND ")";
    break;
  default:
    break;
  }
#endif

  fprintf(stderr, "%s%s: %s\n", categoryStr.data(), severityStr.data(),
          msg.toLocal8Bit().data());
}
