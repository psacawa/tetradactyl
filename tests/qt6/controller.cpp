// Copyright 2023 Paweł Sacawa. All rights reserved.

#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>
#include <QtTest>

#include <qobject.h>
#include <qt6/controller.h>
#include <qt6/logging.h>
#include <qt6/overlay.h>
#include <qtestkeyboard.h>

using Tetradactyl::Controller;
using Tetradactyl::Overlay;
using Tetradactyl::WindowController;

class ControllerTest : public QObject {
  Q_OBJECT
private slots:
  void init();
  void cleanup();
  void testHintActivate();
  void testControllersAndOverlayCreation();
  void testHintFocusInput();

private:
  QWidget *win;
  const Controller *controller;
  QVBoxLayout *layout;
  QList<QLineEdit *> lineEdits;
  QList<QPushButton *> buttons;
  QList<QLabel *> labels;
};

// create a basic window with buttons, text inputs, and labels
void ControllerTest::init() {
  win = new QWidget;
  layout = new QVBoxLayout(win);
  for (int i = 0; i != 2; ++i) {
    QLineEdit *lineEdit = new QLineEdit(win);
    lineEdits.append(lineEdit);
    layout->addWidget(lineEdit);

    QPushButton *button = new QPushButton(QString("Button %1").arg(i), win);
    buttons.append(button);
    layout->addWidget(button);

    QLabel *label = new QLabel(QString("Label %1").arg(i), win);
    labels.append(label);
    layout->addWidget(label);
  }

  Controller::createController();
  controller = Controller::instance();
}

void ControllerTest::cleanup() {
  delete win;
  delete controller;
}

void ControllerTest::testControllersAndOverlayCreation() {
  const Controller *controller = Controller::instance();
  QVERIFY2(controller != nullptr,
           "Tetradactyl::Controller instance was created");
  QVERIFY2(controller->windows().length() == 1,
           "One Tetradactyl::WindowController was created");
  QList<Overlay *> overlays = win->findChildren<Overlay *>();
  QVERIFY2(overlays.length() == 1, "One Tetradactyl::Overlay was created");
  Overlay *overlay = overlays.at(0);
  QVERIFY2(overlay->parent() == win, "Overlay parent is window widget");
}

void ControllerTest::testHintActivate() {
  WindowController *windowController = controller->windows().at(0);
  Overlay *overlay = windowController->overlays().at(0);
  QTest::keyClick(win, Qt::Key_F);
  QObjectList overlayChildren = overlay->children();
  win->show();
  QTRY_COMPARE(overlayChildren.length(), 2);
  // TODO 10/09/20 psacawa: finish this
}

void ControllerTest::testHintFocusInput() {}

QTEST_MAIN(ControllerTest);
#include "controller.moc"
