// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QByteArray>
#include <QDebug>
#include <QMap>

#include "logging.h"

QMap<const char *, Qt::GlobalColor> lcColorMap = {};

static QMap<Qt::GlobalColor, QByteArray> ansiTermColorMap = {
    {Qt::red, RED_INTENSE},   {Qt::green, GREEN_INTENSE},
    {Qt::blue, BLUE_INTENSE}, {Qt::yellow, YELLOW_INTENSE},
    {Qt::cyan, CYAN_INTENSE}, {Qt::magenta, MAGENTA_INTENSE},
};

void colorMessageHandler(QtMsgType severity, const QMessageLogContext &ctx,
                         const QString &msg) {

  QByteArray prefix, suffix, severityStr;
  Qt::GlobalColor color =
      lcColorMap.value(ctx.category, Qt::GlobalColor::color0);
  if (color != Qt::GlobalColor::color0) {
    prefix = ansiTermColorMap.value(color);
    suffix = RESET_FOREGROUND;
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

  fprintf(stderr, "%s%s%s%s: %s\n", prefix.data(), ctx.category, suffix.data(),
          severityStr.data(), msg.toLocal8Bit().data());
}
