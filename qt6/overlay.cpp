// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QLoggingCategory>
#include <QPainter>
#include <iterator>
#include <qassert.h>

#include <algorithm>
#include <iterator>
#include <qobject.h>

#include "hint.h"
#include "logging.h"
#include "overlay.h"

using std::find_if, std::copy_if;

LOGGING_CATEGORY_COLOR("tetradactyl.overlay", Qt::yellow);

namespace Tetradactyl {

Overlay::Overlay(QWidget *target) : QWidget(target), p_selectedHint(nullptr) {
  Q_ASSERT(target != nullptr);
  // Since Overlay is not in any layout, this is needed.
  setFixedSize(2000, 2000);
  hide();
}

// Possible to do this nonsense statically?
QList<HintLabel *> Overlay::hints() {
  QList<HintLabel *> ret;
  ret.reserve(children().size());
  for (auto child : children())
    ret.append(qobject_cast<HintLabel *>(child));
  return ret;
}
QList<HintLabel *> Overlay::visibleHints() {
  auto allHints = hints();
  QList<HintLabel *> ret;
  copy_if(allHints.begin(), allHints.end(), std::back_inserter(ret),
          [](HintLabel *hint) { return hint->isVisible(); });
  return ret;
}

void Overlay::addHint(QString text, QWidget *target) {
  HintData data = {new HintLabel(text, target, this),
                   target->mapTo(parentWidget(), QPoint(0, 0))};
  data.label->show();
  hintData.append(data);
  update();
}

void Overlay::nextHint(bool forward) {
  logInfo << __PRETTY_FUNCTION__;
  Q_ASSERT(hintData.length() > 0);
  QList<HintData>::iterator selectedData =
      find_if(hintData.begin(), hintData.end(),
              [this](HintData &data) { return data.label == p_selectedHint; });
  if (selectedData == hintData.end()) {
    Q_ASSERT(p_selectedHint == nullptr);
    p_selectedHint = hintData.at(0).label;
  } else {
    p_selectedHint->setSelected(false);
    int idx = std::distance(hintData.begin(), selectedData);
    Q_ASSERT(idx < hintData.length());
    int step = forward ? 1 : -1;
    // Work around C's stupid negative modulus
    do {
      idx += step;
      if (idx == hintData.length())
        idx = 0;
      else if (idx < 0)
        idx = hintData.length() - 1;
      p_selectedHint = hintData.at(idx).label;
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
  } else if (hintData.length() > 0) {
    p_selectedHint = hintData.at(0).label;
    p_selectedHint->setSelected(true);
  }
}

void Overlay::removeHint(HintLabel *hint) {
  Q_ASSERT(hint->parentWidget() == this);
  // int index = hints.indexOf(hint);
  const auto &hintDatum =
      find_if(hintData.begin(), hintData.end(),
              [hint](HintData &data) { return data.label == hint; });
  if (hintDatum != hintData.end()) {
    int index = hintDatum - hintData.begin();
    Q_ASSERT(index >= 0);
    hintData.remove(index);
  } else {
    logWarning << hint << "not a hint controlled by" << this;
  }
  update();
}

void Overlay::clear() {
  for (auto data : hintData) {
    delete data.label;
  }
  hintData.clear();
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
  for (auto hint : hintData) {
    if (hint.label->text().startsWith(buffer)) {
      numHintsVisible++;
      hint.label->show();
    } else {
      hint.label->hide();
    }
  }
  if (!p_selectedHint->isVisible()) {
    // Reset the selected hint to the first visible one, if possible.
    if (numHintsVisible == 0) {
      p_selectedHint = nullptr;
    } else {
      for (auto hint : hintData) {
        if (hint.label->isVisible()) {
          resetSelection(hint.label);
          break;
        }
      }
    }
  }
  update();

  return numHintsVisible;
}

void Overlay::paintEvent(QPaintEvent *event) {
  logInfo << __PRETTY_FUNCTION__ << event;
  QPainter p(this);
  for (auto hint : hintData) {
    if (hint.label->isVisible()) {
      hint.label->render(&p, mapTo(parentWidget(), hint.position));
    }
  }
}

} // namespace Tetradactyl
