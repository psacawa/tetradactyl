// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QAbstractItemView>
#include <QHeaderView>
#include <QListView>
#include <QRect>
#include <QTableView>
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
    if ((flags & itemFlag) == flags) {
      logDebug << "Hinting" << instance << "at" << idx << cellRect.topLeft();
      QTableViewActionProxy *proxy =
          new QTableViewActionProxy(idx, cellRect.topLeft(), instance);
      proxies.append(proxy);
    }
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

// QTableViewActionProxy

static void tableViewHintHelper(BaseAction *action, QWidget *widget,
                                QList<QWidgetActionProxy *> &proxies,
                                Qt::ItemFlags itemFlag) {
  QTableView *instance = qobject_cast<QTableView *>(widget);
  QAbstractItemModel *model = instance->model();
  int horHeaderHeight = instance->horizontalHeader()->height();
  int vertHeaderWidth = instance->verticalHeader()->width();
  // For some reason the visualRect is  given excluding the effect of headers
  QPoint headerOffset(vertHeaderWidth, horHeaderHeight);
  QModelIndex topLeftIndex = instance->indexAt(QPoint(0, 0));
  logDebug << "Boundary indices of" << instance << topLeftIndex;

  auto isVisible = [instance](QRect cellRect) {
    QRect visibleRect = instance->childrenRect().intersected(cellRect);
    return !visibleRect.isEmpty();
  };
  for (int row = topLeftIndex.row();; ++row) {
    QModelIndex idx =
        model->index(row, topLeftIndex.column(), topLeftIndex.parent());
    if (!idx.isValid() || (instance->visualRect(idx)).isEmpty())
      break;
    for (int column = topLeftIndex.column();; ++column) {
      QModelIndex idx = model->index(row, column, topLeftIndex.parent());
      QRect cellRect = instance->visualRect(idx);
      // For some reason the visualRect is  given excluding the effect of
      // headers
      cellRect.translate(headerOffset);
      if (!idx.isValid() || !isVisible(cellRect))
        break;
      // We are in the viewport
      auto flags = model->flags(idx);
      if ((flags & itemFlag) == itemFlag) {
        logDebug << "Hinting" << instance << "at" << idx << cellRect.topLeft();
        QTableViewActionProxy *proxy =
            new QTableViewActionProxy(idx, cellRect.topLeft(), instance);
        proxies.append(proxy);
      }
    }
  }
}

void QTableViewActionProxyStatic::hintEditable(
    EditAction *action, QWidget *widget, QList<QWidgetActionProxy *> &proxies) {
  tableViewHintHelper(action, widget, proxies,
                      Qt::ItemIsEditable | Qt::ItemIsEnabled);
}

void QTableViewActionProxyStatic::hintFocusable(
    FocusAction *action, QWidget *widget,
    QList<QWidgetActionProxy *> &proxies) {
  tableViewHintHelper(action, widget, proxies, Qt::ItemIsEnabled);
}

} // namespace Tetradactyl
