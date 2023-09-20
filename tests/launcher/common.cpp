// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QtTest>
#include <random>

#include "common.h"

using namespace std;

string randomString(int max_length) {
  string possible_characters =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  random_device rd;
  mt19937 engine(rd());
  uniform_int_distribution<> dist(0, possible_characters.size() - 1);
  string ret = "";
  for (int i = 0; i < max_length; i++) {
    int random_index =
        dist(engine); // get index between 0 and possible_characters.size()-1
    ret += possible_characters[random_index];
  }
  return ret;
}

QDir tempTestDir() {
  QString tempDirString = QString("/tmp/tetradactylTestDir/%1/%2")
                              .arg(QTest::currentTestFunction())
                              .arg(QString::fromStdString(randomString(6)));
  QDir dir(tempDirString);
  dir.mkpath(".");
  return dir;
}
