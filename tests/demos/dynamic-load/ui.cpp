// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

extern "C" void runUi(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QWidget widget;
  QVBoxLayout layout(&widget);
  QLabel label("hello", &widget);
  QPushButton button1("Button 1");
  QPushButton button2("Button 2");
  layout.addWidget(&label);
  layout.addWidget(&button1);
  layout.addWidget(&button2);
  widget.show();
  app.exec();
}
