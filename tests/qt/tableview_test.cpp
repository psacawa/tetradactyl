// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QModelIndex>
#include <QPoint>
#include <QSignalSpy>
#include <QTableView>
#include <QTableWidget>
#include <QTest>

#include <qt/controller.h>
#include <qt/hint.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/layouts/basiclayouts/basiclayouts
#include <spreadsheet/spreadsheet.h>

namespace Tetradactyl {

class TableViewTest : public QtBaseTest {
  Q_OBJECT

  SpreadSheet *win;
  QTableWidget *table;
  QAbstractItemModel *model;
  QItemSelectionModel *selectionModel;
  QSignalSpy *activatedSpy, *enteredSpy;
  QSignalSpy *currentChangedSpy;

private slots:
  void init();
  void cleanup();
  void basicFocusTest();
  void basicEditTest();
};

void TableViewTest::init() {
  win = new SpreadSheet(10, 6);
  table = win->findChild<QTableWidget *>();
  model = table->model();
  selectionModel = table->selectionModel();
  activatedSpy = new QSignalSpy(table, &QTableWidget::activated);
  enteredSpy = new QSignalSpy(table, &QTableWidget::entered);
  currentChangedSpy =
      new QSignalSpy(selectionModel, &QItemSelectionModel::currentChanged);
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
}

void TableViewTest::cleanup() {
  delete win;
  delete controller;
}

void TableViewTest::basicFocusTest() {
  pressKeys(";");
  QList<const QMetaObject *> mos = {&QLineEdit::staticMetaObject,
                                    &QTableWidget::staticMetaObject};
  for (auto hint : windowController->activeOverlay()->hints())
    QVERIFY2(mos.contains(hint->target->metaObject()),
             "hinted are QTableWidget and QLineEdit");

  // select cell B1
  pressKeys("aad");
  QCOMPARE(acceptedSpy->count(), 1);
  QCOMPARE(currentChangedSpy->count(), 2);
  QModelIndex idx =
      qvariant_cast<QModelIndex>(currentChangedSpy->takeAt(0).at(0));
  auto item = table->currentItem();
  QCOMPARE(QPoint(item->row(), item->column()), QPoint(0, 1));
}

void TableViewTest::basicEditTest() {
  pressKeys("gi");
  QCOMPARE(activatedSpy->count(), 0);
  QCOMPARE(enteredSpy->count(), 0);

  // select cell C1
  pressKeys("aaf");
  QVERIFY(table->isPersistentEditorOpen(model->index(0, 2)));
}
} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::TableViewTest);
#include "tableview_test.moc"
