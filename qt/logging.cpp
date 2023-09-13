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

void colorMessageHandler(QtMsgType type, const QMessageLogContext &ctx,
                         const QString &msg) {

  QByteArray prefix, suffix;
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
  fprintf(stderr, "%s%s%s: %s\n", prefix.data(), ctx.category, suffix.data(),
          msg.toLocal8Bit().data());
}
