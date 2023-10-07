// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QSignalSpy>
#include <QTest>
#include <QTreeView>

#include <qt/controller.h>
#include <qt/hint.h>
#include <qt/overlay.h>

#include "common.h"

// from "git://qtbase/examples/widgets/itemviews/editabletreemodel
#include <editabletreemodel/mainwindow.h>

namespace Tetradactyl {

class TreeViewTest : public QtBaseTest {
  Q_OBJECT

  MainWindow *win;
  QTreeView *tree;
  QAbstractItemModel *model;
  QItemSelectionModel *selection;

private slots:
  void init();
  void cleanup();
  void basicExpandTest();
  void basicFocusTest();
  void basicEditTest();
};

void TreeViewTest::init() {
  win = new MainWindow;
  tree = win->findChild<QTreeView *>();
  model = tree->model();
  selection = tree->selectionModel();
  QtBaseTest::init();
  waitForWindowActiveOrFail(win);
}

void TreeViewTest::cleanup() {
  delete win;
  delete controller;
}

void TreeViewTest::basicExpandTest() {
  pressKeys("f");
  QModelIndex formEditingModeIndex = model->index(3, 0);
  QVERIFY(!tree->isExpanded(formEditingModeIndex));

  // expand "Form Editing Mode"
  pressKeys("al");
  QCOMPARE(acceptedSpy->count(), 1);
  QVERIFY(tree->isExpanded(formEditingModeIndex));

  pressKeys("f");
  QModelIndex layoutsIndex = model->index(4, 0, formEditingModeIndex);
  QVERIFY(!tree->isExpanded(layoutsIndex));

  // expand "Layouts"
  pressKeys("ds");
  QCOMPARE(acceptedSpy->count(), 2);
  QVERIFY(tree->isExpanded(layoutsIndex));
}

void TreeViewTest::basicFocusTest() {
  pressKeys(";");
  pressKeys("as");

  QCOMPARE(selection->currentIndex(), model->index(0, 1));
}

void TreeViewTest::basicEditTest() {
  pressKeys("gi");
  pressKeys("as");
  QVERIFY(tree->isPersistentEditorOpen(model->index(0, 1)));
}

} // namespace Tetradactyl

QTEST_MAIN(Tetradactyl::TreeViewTest);
#include "treeview_test.moc"
