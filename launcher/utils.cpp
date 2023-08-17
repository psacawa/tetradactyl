#include <QList>
#include <QMetaEnum>
#include <QString>

#include <unistd.h>

#include "utils.h"

using std::string;

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

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
  int bytes = MIN(readlink("/proc/self/exe", pathBuf, len), len - 1);
  if (bytes >= 0)
    pathBuf[bytes] = '\0';
  return QString(pathBuf);
}
