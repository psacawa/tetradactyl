#include <QList>
#include <QMetaEnum>
#include <QString>

#include "utils.h"

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
