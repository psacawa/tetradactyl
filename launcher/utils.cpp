// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDir>

#include <exception>
#include <unistd.h>

#include "utils.h"

using std::runtime_error;
using std::string;

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

void system_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

QList<QString> keysFromEnum(QMetaEnum &e_num) {
  QList<QString> ret;
  for (int i = 0; i != e_num.keyCount(); ++i) {
    ret.push_back(e_num.key(i));
  }
  return ret;
}

QString join(QList<QString> arr, QString sep) {
  QString ret = "";
  for (int i = 0; i != arr.length(); ++i) {
    if (i)
      ret += sep;
    ret += arr[i];
  }
  return ret;
}

QString getLocationOfThisProgram() {
  char pathBuf[256];
  size_t len = sizeof(pathBuf);
  int bytes = MIN(readlink("/proc/self/exe", pathBuf, len), (ssize_t)len - 1);
  if (bytes >= 0)
    pathBuf[bytes] = '\0';
  return QString(pathBuf);
}

void mkdirRec(QDir dir, QString relPath) {
  if (!dir.exists())
    throw runtime_error(qPrintable(u"dir %1 doesn't exist"_qs.arg(dir.path())));

  auto parts = relPath.split('/');
  for (auto part : parts) {
    if (!dir.exists(part)) {
      bool success = dir.mkdir(part);
      if (!success)
        throw runtime_error(
            qPrintable(u"couldn't write to  %1"_qs.arg(dir.path())));
      success = dir.cd(part);
      if (!success)
        throw runtime_error(qPrintable(u"couldn't cd to  %1"_qs.arg(part)));
    }
  }
}
