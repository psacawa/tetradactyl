// Copyright 2023 Paweł Sacawa. All rights reserved.

#include <QClipboard>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QPushButton>
#include <QSignalSpy>
#include <QVBoxLayout>
#include <QWindow>
#include <QtTest>
#include <qt6/QtCore/qglobal.h>
#include <stdlib.h>

#include <qnamespace.h>
#include <qobject.h>
#include <qtestcase.h>
#include <qtestkeyboard.h>
#include <qtestsupport_core.h>
#include <qtestsupport_widgets.h>
#include <qwidget.h>

#include "common.h"
#include <qt/controller.h>
#include <qt/hint.h>
#include <qt/logging.h>
#include <qt/overlay.h>

#define NUM_BUTTONS 10
#define NUM_LINEEDITS 2
#define NUM_LABELS 2

using Tetradactyl::Controller;
using Tetradactyl::HintLabel;
using Tetradactyl::Overlay;
using Tetradactyl::WindowController;

class BasicControllerTest : public QObject {
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
  void testHintYank();
  void testHintFocus();

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

void BasicControllerTest::initTestCase() {
  // QLoggingCategory::setFilterRules(QStringLiteral("tetradactyl.*.info=false"));
}

// create a basic window with buttons, text inputs, and labels
void BasicControllerTest::init() {
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
  Tetradactyl::waitForWindowActiveOrFail(win);
}

void BasicControllerTest::cleanup() {
  delete controller;
  delete win;
  buttons.clear();
  lineEdits.clear();
  labels.clear();
}

void BasicControllerTest::testControllersAndOverlayCreation() {
  QVERIFY2(controller != nullptr,
           "Tetradactyl::Controller instance was created");
  QVERIFY2(controller->windows().length() == 1,
           "One Tetradactyl::WindowController was created");
  QList<Overlay *> overlays = win->findChildren<Overlay *>();
  QVERIFY2(overlays.length() == 1, "One Tetradactyl::Overlay was created");
  Overlay *overlay = overlays.at(0);
  QVERIFY2(overlay->parent() == win, "Overlay parent is window widget");
  QVERIFY(!overlay->isVisible());
}

void BasicControllerTest::testHintActivate() {
  QTest::keyClick(win, Qt::Key_F);
  QList<HintLabel *> overlayChildren = overlay->hints();
  QTRY_VERIFY2(overlayChildren.length() == NUM_BUTTONS,
               "Activating fires underlying widgets signal");
  QVERIFY(overlay->isVisible());

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
  overlayChildren = overlay->hints();
  QTRY_VERIFY2(overlayChildren.length() == 0, "Activating removes hint labels");
  QVERIFY2(!overlay->isVisible(), "Overlay invisible after hint accepted");
}

void BasicControllerTest::testHintCancel() {
  QTest::keyClick(win, Qt::Key_F);
  QTRY_COMPARE(overlay->hints().length(), NUM_BUTTONS);
  QTest::keyClick(win, Qt::Key_Escape);
  QTRY_COMPARE(overlay->hints().length(), 0);
  QVERIFY2(!overlay->isVisible(), "Overlay invisible after hinting canceled");

  // TODO 12/09/20 psacawa: finish this
}

void BasicControllerTest::testHintVisibilityAfterPushPopKey() {

  // hint with F
  QTest::keyClick(win, Qt::Key_F);
  QCOMPARE(overlay->visibleHints().length(), NUM_BUTTONS);

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

void BasicControllerTest::testHintNextHint() {
  QTest::keyClick(win, Qt::Key_F);
  QTest::keyClick(win, Qt::Key_S);
  // 3 of 10 hints visible
  auto visibleHints = overlay->visibleHints();
  QCOMPARE(visibleHints.length(), 3);
  QVERIFY2(overlay->selectedHint() == visibleHints.at(0),
           "selected hint starts at first visible");
  QTest::keyClick(win, Qt::Key_Tab);
  QVERIFY2(overlay->selectedHint() == visibleHints.at(1),
           "tab cycles selected hint");
  QTest::keyClick(win, Qt::Key_Tab);
  QCOMPARE(overlay->selectedHint(), visibleHints.at(2));
  QTest::keyClick(win, Qt::Key_Tab);
  QVERIFY2(overlay->selectedHint() == visibleHints.at(0),
           "tab wraps froward to the first visible hint");
  QTest::keyClick(win, Qt::Key_Backtab);
  int selectedHintIdx = visibleHints.indexOf(overlay->selectedHint());
  QVERIFY2(selectedHintIdx == 2,
           "shift+tab wraps backward to the last visible hint");
  QTest::keyClick(win, Qt::Key_Backtab);
  QVERIFY2(overlay->selectedHint() == visibleHints.at(1),
           "shift+tab wraps backward to the last visible hint");
}

void BasicControllerTest::testHintFocusInput() {
  QTest::keyClick(win, Qt::Key_G);
  QTest::keyClick(win, Qt::Key_I);
  QList<HintLabel *> hints = overlay->visibleHints();
  QVERIFY2(hints.length() == NUM_LINEEDITS,
           "Hinting FocusInput hints QLineEdit");
  QVERIFY(overlay->isVisible());
  for (auto hint : hints)
    QVERIFY2(hint->target->metaObject() == &QLineEdit::staticMetaObject,
             "Hinting FocusInput hints QLineEdit");
  QVERIFY2(hints.at(1)->text() == "S", "Hint for focus input is single letter");

  QWidget *target = hints.at(1)->target;

  QTest::keyClick(win, Qt::Key_S);
  QVERIFY(!overlay->isVisible());
  QVERIFY2(win->focusWidget() == target,
           "Accepting focus input action set focus to selected QLineEdit");
}
void BasicControllerTest::testHintYank() {
  QTest::keyClick(win, Qt::Key_Y);
  QVERIFY(overlay->isVisible());
  auto hints = overlay->hints();
  QCOMPARE(hints.length(), NUM_BUTTONS + NUM_LABELS);
  for (auto hint : hints)
    QVERIFY2(hint->target->metaObject() == &QLabel::staticMetaObject ||
                 hint->target->metaObject() == &QPushButton::staticMetaObject,
             "The hints in yank mode point to QLabel or QPushButton instances");
  QClipboard *clipboard = QGuiApplication::clipboard();
  QSignalSpy clipboardChangedSpy(clipboard, &QClipboard::changed);
  QTest::keyClick(win, Qt::Key_A);
  QTest::keyClick(win, Qt::Key_A);
  QCOMPARE(clipboardChangedSpy.count(), 1);
  qInfo() << clipboard->text();
  // label0 text copied
  QCOMPARE(clipboard->text(), buttons.at(0)->text());

  QVERIFY(!overlay->isVisible());
  QCOMPARE(overlay->hints().length(), 0);
}
void BasicControllerTest::testHintFocus() {
  QTest::keyClick(win, Qt::Key_Semicolon);
  QCOMPARE(overlay->hints().length(), NUM_BUTTONS + NUM_LINEEDITS);
  for (auto hint : overlay->hints()) {
    qInfo() << hint->target;
    QVERIFY2(hint->target->metaObject() == &QPushButton::staticMetaObject ||
                 hint->target->metaObject() == &QLineEdit::staticMetaObject,
             "hinted focusable elements are buttons and QLineEdit  ");
  }
  QWidget *secondFocusableWidget = buttons.at(1);
  QTest::keyClick(win, Qt::Key_A);
  QTest::keyClick(win, Qt::Key_S);
  QVERIFY(!overlay->isVisible());
  QVERIFY2(secondFocusableWidget->hasFocus(),
           "Accepted widget for HintMode::Focusable has focus set");
}

QTEST_MAIN(BasicControllerTest);
#include "basiccontroller_test.moc"
