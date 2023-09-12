// Copyright 2023 Paweł Sacawa. All rights reserved.

#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QSignalSpy>
#include <QVBoxLayout>
#include <QWindow>
#include <QtTest>

#include <qnamespace.h>
#include <qobject.h>
#include <qt6/controller.h>
#include <qt6/logging.h>
#include <qt6/overlay.h>
#include <qtestcase.h>
#include <qtestkeyboard.h>
#include <qtestsupport_core.h>
#include <qtestsupport_widgets.h>

#include "common.h"
#include "qt6/hint.h"

#define NUM_BUTTONS 10
#define NUM_LINEEDITS 2
#define NUM_LABELS 2

using Tetradactyl::Controller;
using Tetradactyl::HintLabel;
using Tetradactyl::Overlay;
using Tetradactyl::WindowController;

class ControllerTest : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();
  void init();
  void cleanup();
  void testHintActivate();
  void testHintCancel();
  void testHintVisibilityAfterPushPopKey();
  void testHintNextHint();
  void testControllersAndOverlayCreation();
  void testHintFocusInput();

private:
  QWidget *win;
  const Controller *controller;
  QVBoxLayout *layout;
  QList<QLineEdit *> lineEdits;
  QList<QPushButton *> buttons;
  QList<QLabel *> labels;
  WindowController *windowController;
  Overlay *overlay;
};

void ControllerTest::initTestCase() {
  // QLoggingCategory::setFilterRules(QStringLiteral("tetradactyl.*.info=false"));
}

// create a basic window with buttons, text inputs, and labels
void ControllerTest::init() {
  win = new QWidget;
  layout = new QVBoxLayout(win);
  for (int i = 0; i != NUM_BUTTONS; ++i) {
    QPushButton *button = new QPushButton(QString("Button %1").arg(i), win);
    buttons.append(button);
    layout->addWidget(button);
  }
  for (int i = 0; i != NUM_LINEEDITS; ++i) {
    QLineEdit *lineEdit = new QLineEdit(win);
    lineEdits.append(lineEdit);
    layout->addWidget(lineEdit);
  }
  for (int i = 0; i != NUM_LABELS; ++i) {
    QLabel *label = new QLabel(QString("Label %1").arg(i), win);
    labels.append(label);
    layout->addWidget(label);
  }
  Controller::createController();
  controller = Controller::instance();
  windowController = controller->windows().at(0);
  overlay = windowController->overlays().at(0);
}

void ControllerTest::cleanup() {
  delete controller;
  delete win;
}

void ControllerTest::testControllersAndOverlayCreation() {
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
  waitForWindowActiveOrFail(win);
  QTest::keyClick(win, Qt::Key_F);
  QObjectList overlayChildren = overlay->children();
  QTRY_VERIFY2(overlayChildren.length() == NUM_BUTTONS,
               "Activating fires underlying widgets signal");

  HintLabel *label;
  label = qobject_cast<HintLabel *>(overlayChildren.at(0));
  // base test settings have hintChars == "ASDFJKL", so with 7 < NUM_BUTTONS <
  // 50,  we have hints of length 2
  QCOMPARE(label->text(), "AA");
  label = qobject_cast<HintLabel *>(overlayChildren.at(NUM_BUTTONS - 1));
  QCOMPARE(label->text(), "SD");

  // simple activation
  QSignalSpy clickedSpy(buttons.at(0), &QPushButton::clicked);
  QTest::keyClick(win, Qt::Key_A);
  QVERIFY2(clickedSpy.count() == 0, "One keypress doesn't narrow down hint");
  QTest::keyClick(win, Qt::Key_A);
  QVERIFY2(clickedSpy.count() == 1,
           "Activating fires underlying widgets signal");
  overlayChildren = overlay->children();
  QTRY_VERIFY2(overlayChildren.length() == 0, "Activating removes hint labels");
}

void ControllerTest::testHintCancel() {
  waitForWindowActiveOrFail(win);
  QTest::keyClick(win, Qt::Key_F);
  QTRY_COMPARE(overlay->hints().length(), NUM_BUTTONS);
  QTest::keyClick(win, Qt::Key_Escape);

  // TODO 12/09/20 psacawa: finish this
}

void ControllerTest::testHintVisibilityAfterPushPopKey() {
  waitForWindowActiveOrFail(win);

  // hint with F
  QTest::keyClick(win, Qt::Key_F);
  QCOMPARE(overlay->children().length(), NUM_BUTTONS);

  QList<HintLabel *> visibleHints = overlay->visibleHints();
  QCOMPARE(visibleHints.length(), NUM_BUTTONS);

  for (int i = 0; i != NUM_BUTTONS; ++i) {
    HintLabel *label = overlay->hints().at(i);
    QCOMPARE(label->isSelected(), i == 0 ? true : false);
  }
  // TODO 12/09/20 psacawa: test selected/deselected color via
  // QPalette->QBrush->QColor

  // filter with S
  QTest::keyClick(win, Qt::Key_S);
  visibleHints = overlay->visibleHints();
  QVERIFY2(visibleHints.length() == 3,
           "after pressing S, only 3 hints remain visible");
  QVERIFY2(overlay->selectedHint()->text() == "SA", "selectedHint was updated");
  QCOMPARE(overlay->selectedHint(), overlay->hints().at(7));
  QVERIFY2(overlay->selectedHint() == visibleHints.at(0),
           "selectedHint is first visible hint");

  // backspace
  QTest::keyClick(win, Qt::Key_Backspace);
  visibleHints = overlay->visibleHints();
  QVERIFY2(visibleHints.length() == NUM_BUTTONS,
           "after backspace, all hints visible");
  QVERIFY2(overlay->selectedHint()->text() == "SA", "selectedHint is the same");

  // another backspace - no effect
  QTest::keyClick(win, Qt::Key_Backspace);
  visibleHints = overlay->visibleHints();
  QVERIFY2(visibleHints.length() == NUM_BUTTONS,
           "backspace with empty hint buffer has no effect");
  QVERIFY2(overlay->selectedHint()->text() == "SA", "selectedHint is the same");
}

void ControllerTest::testHintNextHint() {
  // TODO 12/09/20 psacawa: finish this
}

void ControllerTest::testHintFocusInput() {
  // TODO 12/09/20 psacawa: finish this
}

QTEST_MAIN(ControllerTest);
#include "controller.moc"
