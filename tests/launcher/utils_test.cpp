// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QObject>
#include <QTemporaryDir>
#include <QtTest>

#include <launcher/utils.h>

namespace Tetradactyl {

class UtilsTest : public QObject {
  Q_OBJECT
public:
private slots:
  void mkdirRecTest();
};

void UtilsTest::mkdirRecTest() {
  QTemporaryDir tempDir;
  QDir dir(tempDir.path());
  QVERIFY(tempDir.isValid());
  mkdirRec(dir, ".local/share/tetradactyl");

  for (auto part : u".local/share/tetradactyl"_qs.split('/')) {
    QVERIFY(dir.exists(part));
    dir.cd(part);
  }
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::UtilsTest)
#include "utils_test.moc"
