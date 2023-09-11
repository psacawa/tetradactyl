// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QLoggingCategory>
#include <iterator>
#include <qassert.h>

#include <algorithm>

#include "hint.h"
#include "logging.h"
#include "overlay.h"

using std::find_if;

LOGGING_CATEGORY_COLOR("tetradactyl.overlay", Qt::yellow);

namespace Tetradactyl {

Overlay::Overlay(QWidget *target) : QWidget(target), selectedHint(nullptr) {
  Q_ASSERT(target != nullptr);
}

void Overlay::addHint(QString text, QWidget *target) {
  HintData data = {new HintLabel(text, target),
                   target->mapTo(parentWidget(), target->pos())};
  data.label->show();
  hints.append(data);
}

void Overlay::nextHint(bool forward) {
  Q_ASSERT(hints.length() > 0);
  QList<HintData>::iterator selectedData =
      find_if(hints.begin(), hints.end(),
              [this](HintData &data) { return data.label == selectedHint; });
  if (selectedData == hints.end()) {
    Q_ASSERT(selectedHint == nullptr);
    selectedHint = hints.at(0).label;
  } else {
    selectedHint->setSelected(false);
    int idx = std::distance(hints.begin(), selectedData);
    Q_ASSERT(idx < hints.length());
    int step = forward ? 1 : -1;
    // Work around C's stupid negative modulus
    do {
      idx += step;
      if (idx == hints.length())
        idx = 0;
      else if (idx < 0)
        idx = hints.length() - 1;
      selectedHint = hints.at(idx).label;
    } while (!selectedHint->isVisible());
  }
  selectedHint->setSelected(true);
}

void Overlay::resetSelection(HintLabel *label) {
  // Were we pointing at anything before?
  if (selectedHint != nullptr)
    selectedHint->setSelected(false);
  if (label) {
    selectedHint = label;
    selectedHint->setSelected(true);
  } else if (hints.length() > 0) {
    selectedHint = hints.at(0).label;
    selectedHint->setSelected(true);
  }
}

void Overlay::removeHint(HintLabel *hint) {
  Q_ASSERT(hint->parentWidget() == this);
  // int index = hints.indexOf(hint);
  const auto &hintData =
      find_if(hints.begin(), hints.end(),
              [hint](HintData &data) { return data.label == hint; });
  if (hintData != hints.end()) {
    int index = hintData - hints.begin();
    Q_ASSERT(index >= 0);
    hints.remove(index);
  } else {
    logWarning << hint << "not a hint controlled by" << this;
  }
}

void Overlay::clear() {
  for (auto data : hints) {
    delete data.label;
  }
  hints.clear();
  selectedHint = nullptr;
}

QWidget *Overlay::selected() {
  return selectedHint != nullptr ? selectedHint->target : nullptr;
}

// Update hint visibility. Return number of visible hints.
int Overlay::updateHints(QString &buffer) {
  int numHintsVisible = 0;
  for (auto hint : hints) {
    if (hint.label->text().startsWith(buffer)) {
      numHintsVisible++;
      hint.label->show();
    } else {
      hint.label->hide();
    }
  }
  if (!selectedHint->isVisible()) {
    // Reset the selected hint to the first visible one, if possible.
    if (numHintsVisible == 0) {
      selectedHint = nullptr;
    } else {
      for (auto hint : hints) {
        if (hint.label->isVisible()) {
          resetSelection(hint.label);
          break;
        }
      }
    }
  }

  return numHintsVisible;
}

} // namespace Tetradactyl
