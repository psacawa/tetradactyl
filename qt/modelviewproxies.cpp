// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractItemView>
#include <QComboBox>
#include <QHeaderView>
#include <QListView>
#include <QRect>
#include <QTableView>
#include <QTreeView>
#include <qobject.h>

#include "action.h"
#include "logging.h"

LOGGING_CATEGORY_COLOR("tetradactyl.modelviewproxies", Qt::cyan);

namespace Tetradactyl {

// QAbstractItemViewActionProxy

bool QAbstractItemViewActionProxy::edit(EditAction *action) {
  QAbstractItemView *instance = qobject_cast<QAbstractItemView *>(widget);
  instance->edit(this->modelIndex);
  return true;
}

bool QAbstractItemViewActionProxy::focus(FocusAction *action) {
  QAbstractItemView *instance = qobject_cast<QAbstractItemView *>(widget);
  instance->setCurrentIndex(this->modelIndex);
  return true;
}

// QListViewActionProxy

static void listViewHintHelper(BaseAction *action, QWidget *widget,
                               QList<QWidgetActionProxy *> &proxies,
                               Qt::ItemFlags itemFlag) {
  QListView *instance = qobject_cast<QListView *>(widget);
  QAbstractItemModel *model = instance->model();
  QModelIndex topLeftIndex = instance->indexAt(QPoint(0, 0));
  logDebug << "Boundary indices of" << instance << topLeftIndex;

  auto isVisible = [instance](QRect cellRect) {
    QRect visibleRect = instance->childrenRect().intersected(cellRect);
    return !visibleRect.isEmpty();
  };
  for (int row = topLeftIndex.row();; ++row) {
    QModelIndex idx = model->index(row, 0, topLeftIndex.parent());
    QRect cellRect = instance->visualRect(idx);
    // For some reason the visualRect is  given excluding the effect of
    // headers
    if (!idx.isValid() || !isVisible(cellRect))
      break;
    // We are in the viewport
    auto flags = model->flags(idx);
    if ((flags & itemFlag) == itemFlag) {
      logDebug << "Hinting" << instance << "at" << idx << cellRect.topLeft();
      QListViewActionProxy *proxy =
          new QListViewActionProxy(idx, cellRect.topLeft(), instance);
      proxies.append(proxy);
    }
  }
}

[[maybe_unused]] static bool isContainedinQComboBox(QWidget *w) {
  w = w->parentWidget();
  while (w != nullptr) {
    if (w->metaObject() == &QComboBox::staticMetaObject)
      return true;
    w = w->parentWidget();
  }
  return false;
}

void QListViewActionProxyStatic::hintActivatable(
    ActivateAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QListView, widget);
  // only interested in hint activating list view when it's a subwidget of
  // QComboBox
  if (isContainedinQComboBox(instance)) {
    listViewHintHelper(action, instance, proxies, Qt::ItemIsEnabled);
  }
}

void QListViewActionProxyStatic::hintEditable(
    EditAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  listViewHintHelper(action, widget, proxies,
                     Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

void QListViewActionProxyStatic::hintFocusable(
    FocusAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  listViewHintHelper(action, widget, proxies, Qt::ItemIsEnabled);
}

// Only helpful for the list descendant of QComboBox and the like
bool QListViewActionProxy::activate(ActivateAction *action) {
  QListView *instance = qobject_cast<QListView *>(widget);
  QComboBox *ancestorComboBox = findAncestor<QComboBox *>(instance);
  Q_ASSERT(ancestorComboBox);
  logInfo << positionInWidget << modelIndex;
  ancestorComboBox->setCurrentIndex(modelIndex.row());
  ancestorComboBox->hidePopup();
  return true;
}

// QTableViewActionProxy

static void tableViewHintHelper(BaseAction *action, QTableView *view,
                                QList<QWidgetActionProxy *> &proxies,
                                Qt::ItemFlags itemFlag) {
  QAbstractItemModel *model = view->model();
  int horHeaderHeight = view->horizontalHeader()->height();
  int vertHeaderWidth = view->verticalHeader()->width();
  // For some reason the visualRect is  given excluding the effect of headers
  QPoint headerOffset(vertHeaderWidth, horHeaderHeight);
  QModelIndex topLeftIndex = view->indexAt(QPoint(0, 0));
  logDebug << "Boundary indices of" << view << topLeftIndex;

  auto isVisible = [view](QRect cellRect) {
    QRect visibleRect = view->childrenRect().intersected(cellRect);
    return !visibleRect.isEmpty();
  };
  for (int row = topLeftIndex.row();; ++row) {
    QModelIndex idx =
        model->index(row, topLeftIndex.column(), topLeftIndex.parent());
    if (!idx.isValid() || (view->visualRect(idx)).isEmpty())
      break;
    for (int column = topLeftIndex.column();; ++column) {
      QModelIndex idx = model->index(row, column, topLeftIndex.parent());
      QRect cellRect = view->visualRect(idx);
      cellRect.translate(headerOffset);
      if (!idx.isValid() || !isVisible(cellRect))
        break;
      // We are in the viewport
      auto flags = model->flags(idx);
      if ((flags & itemFlag) == itemFlag) {
        logDebug << "Hinting" << view << "at" << idx << cellRect.topLeft();
        QTableViewActionProxy *proxy =
            new QTableViewActionProxy(idx, cellRect.topLeft(), view);
        proxies.append(proxy);
      }
    }
  }
}

void QTableViewActionProxyStatic::hintEditable(
    EditAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QTableView, widget);
  tableViewHintHelper(action, instance, proxies,
                      Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

void QTableViewActionProxyStatic::hintFocusable(
    FocusAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QTableView, widget);
  tableViewHintHelper(action, instance, proxies, Qt::ItemIsEnabled);
}

// QTreeViewActionProxy

static void treeViewHintHelper(BaseAction *action, QTreeView *view,
                               QList<QWidgetActionProxy *> &proxies,
                               Qt::ItemFlags itemFlag,
                               QModelIndex root = QModelIndex()) {
  QAbstractItemModel *model = view->model();
  int headerHeight = view->header()->height();
  QPoint headerOffset(0, headerHeight);
  QModelIndex startIndex;
  if (!root.isValid()) {
    // no starting point passed, start recursion at top-left corner of QTreeView
    startIndex = view->indexAt(QPoint(0, 0));
    logInfo << "Boundary indices of" << view << startIndex;
  } else {
    startIndex = model->index(0, 0, root);
  }

  if (view->selectionBehavior() == QAbstractItemView::SelectColumns)
    logWarning << view << "has selection mode" << view->selectionBehavior();

  auto isVisible = [view](QRect cellRect) {
    QRect visibleRect = view->childrenRect().intersected(cellRect);
    return !visibleRect.isEmpty();
  };
  for (int row = startIndex.row();; ++row) {
    QModelIndex idx = model->index(row, startIndex.column(), root);
    if (!idx.isValid() || (view->visualRect(idx)).isEmpty())
      break;
    for (int column = startIndex.column();; ++column) {
      QModelIndex idx = model->index(row, column, startIndex.parent());
      QRect cellRect = view->visualRect(idx);
      cellRect.translate(headerOffset);
      if (!idx.isValid() || !isVisible(cellRect))
        break;
      // We are in the viewport
      auto flags = model->flags(idx);
      if ((flags & itemFlag) == itemFlag) {
        logInfo << "Hinting" << view << "at" << idx << cellRect.topLeft();
        QTreeViewActionProxy *proxy =
            new QTreeViewActionProxy(idx, cellRect.topLeft(), view);
        proxies.append(proxy);
      }
      if (model->hasChildren(idx) && view->isExpanded(idx)) {
        logInfo << "descending into children" << model->data(idx) << idx;
        treeViewHintHelper(action, view, proxies, itemFlag, idx);
      }
      // one loop is enough if we select only rows, as it common
      if (view->selectionBehavior() == QAbstractItemView::SelectRows)
        break;
    }
  }
}

void QTreeViewActionProxyStatic::hintActivatable(
    ActivateAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QTreeView, widget);
  treeViewHintHelper(action, instance, proxies, Qt::ItemIsEnabled);
}

void QTreeViewActionProxyStatic::hintEditable(
    EditAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QTreeView, widget);
  treeViewHintHelper(action, instance, proxies,
                     Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

void QTreeViewActionProxyStatic::hintFocusable(
    FocusAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  QOBJECT_CAST_ASSERT(QTreeView, widget);
  treeViewHintHelper(action, instance, proxies, Qt::ItemIsEnabled);
}

bool QTreeViewActionProxy::activate(ActivateAction *action) {
  QTreeView *instance = qobject_cast<QTreeView *>(widget);
  bool isExpanded = instance->isExpanded(this->modelIndex);
  // FIXME 05/10/20 psacawa: despite appearances this can lead to segfault
  instance->setExpanded(this->modelIndex, !isExpanded);
  return true;
}

} // namespace Tetradactyl
