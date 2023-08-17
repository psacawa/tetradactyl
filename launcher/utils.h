#include <QList>
#include <QMetaEnum>
#include <QString>

QList<QString> keysFromEnum(QMetaEnum &e_num);
QString join(QList<QString> arr, QString sep);
QString getLocationOfThisProgram();
