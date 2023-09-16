// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QLayoutItem>
#include <QLoggingCategory>
#include <QPainter>

#include <qobject.h>

#include <algorithm>
#include <iterator>

#include "action.h"
#include "hint.h"
#include "logging.h"
#include "overlay.h"

using std::copy_if;

LOGGING_CATEGORY_COLOR("tetradactyl.overlay", Qt::yellow);

namespace Tetradactyl {

Overlay::Overlay(QWidget *target) : QWidget(target), p_selectedHint(nullptr) {
  Q_ASSERT(target != nullptr);
  // Since Overlay is not in any layout, this is needed.
  setFixedSize(2000, 2000);
  setLayout(new OverlayLayout(this));
  setAttribute(Qt::WA_TransparentForMouseEvents);
  hide();
}

QList<HintLabel *> Overlay::visibleHints() {
  QList<HintLabel *> ret;
  copy_if(p_hints.begin(), p_hints.end(), std::back_inserter(ret),
          [](HintLabel *hint) { return hint->isVisible(); });
  return ret;
}

void Overlay::addHint(QString text, QWidgetActionProxy *widgetProxy) {
  HintLabel *newHint =
      new HintLabel(text, widgetProxy->widget, this, widgetProxy);
  overlayLayout()->addHint(newHint);
  p_hints.append(newHint);
  update();
}

Overlay::~Overlay() {}

void Overlay::nextHint(bool forward) {
  logInfo << __PRETTY_FUNCTION__;
  Q_ASSERT(p_hints.length() > 0);
  int idx = p_hints.indexOf(selectedHint());
  if (idx < 0) {
    Q_ASSERT(p_selectedHint == nullptr);
    p_selectedHint = p_hints.at(0);
  } else {
    p_selectedHint->setSelected(false);
    Q_ASSERT(idx < p_hints.length());
    int step = forward ? 1 : -1;
    do {
      idx += step;
      if (idx == p_hints.length())
        idx = 0;
      else if (idx < 0)
        idx = p_hints.length() - 1;
      p_selectedHint = p_hints.at(idx);
    } while (!p_selectedHint->isVisible());
  }
  p_selectedHint->setSelected(true);
  update();
}

void Overlay::resetSelection(HintLabel *label) {
  // Were we pointing at anything before?
  if (p_selectedHint != nullptr)
    p_selectedHint->setSelected(false);
  if (label) {
    p_selectedHint = label;
    p_selectedHint->setSelected(true);
  } else if (p_hints.length() > 0) {
    p_selectedHint = p_hints.at(0);
    p_selectedHint->setSelected(true);
  }
}

void Overlay::removeHint(HintLabel *hint) {
  Q_ASSERT(hint->parentWidget() == this);
  int idx = p_hints.indexOf(hint);
  Q_ASSERT(idx >= 0);
  p_hints.removeAt(idx);
  logWarning << hint << "not a hint controlled by" << this;
  update();
}

void Overlay::clear() {
  for (auto hint : p_hints) {
    delete hint;
  }
  p_hints.clear();
  p_selectedHint = nullptr;
  update();
}

HintLabel *Overlay::selectedHint() { return p_selectedHint; }
QWidget *Overlay::selectedWidget() {
  return p_selectedHint != nullptr ? p_selectedHint->target : nullptr;
}

// Update hint visibility. Return number of visible hints.
int Overlay::updateHints(QString &buffer) {
  int numHintsVisible = 0;
  for (auto hint : p_hints) {
    if (hint->text().startsWith(buffer)) {
      numHintsVisible++;
      hint->show();
    } else {
      hint->hide();
    }
  }
  if (!p_selectedHint->isVisible()) {
    // Reset the selected hint to the first visible one, if possible.
    if (numHintsVisible == 0) {
      p_selectedHint = nullptr;
    } else {
      for (auto hint : p_hints) {
        if (hint->isVisible()) {
          resetSelection(hint);
          break;
        }
      }
    }
  }
  update();

  return numHintsVisible;
}

QList<HintLabel *> findHintsByTargetHelper(Overlay *overlay,
                                           const QMetaObject *mo) {
  QList<HintLabel *> ret;
  for (auto hint : overlay->hints()) {
    if (hint->target->metaObject()->inherits(mo))
      ret.append(hint);
  }
  return ret;
}

OverlayLayout::~OverlayLayout() {
  QLayoutItem *item;
  while ((item = takeAt(0)))
    delete item;
}

int OverlayLayout::count() const { return items.length(); }

void OverlayLayout::addHint(HintLabel *hint) { addItem(new QWidgetItem(hint)); }
void OverlayLayout::addItem(QLayoutItem *item) { items.append(item); }

// Perform the layout. Must make sure that hints don't occlude one another or
// escape the window geometry here.
void OverlayLayout::setGeometry(const QRect &updateRect) {
  for (auto item : items) {
    HintLabel *hint = qobject_cast<HintLabel *>(item->widget());
    QRect hintGeometry = QRect(hint->positionInOverlay, hint->sizeHint());
    QRect hostGeometry = overlay()->parentWidget()->geometry();
    if (!hintGeometry.intersects(hintGeometry)) {
      logWarning << "Creating hint " << hint << "at" << hintGeometry
                 << "which doesn't intersect overlay host geometry"
                 << hostGeometry;
    }
    item->setGeometry(hintGeometry);
  }
}

QLayoutItem *OverlayLayout::itemAt(int index) const {
  return items.value(index, nullptr);
}

QLayoutItem *OverlayLayout::takeAt(int index) {
  QLayoutItem *item = items.value(index, nullptr);
  if (item != nullptr) {
    items.removeAt(index);
  }
  return item;
}

QSize OverlayLayout::sizeHint() const { return minimumSize(); };

QSize OverlayLayout::minimumSize() const { return QSize(2000, 2000); };

} // namespace Tetradactyl
