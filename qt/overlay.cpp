// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QDebug>
#include <QLabel>
#include <QLayoutItem>
#include <QLoggingCategory>
#include <QPainter>
#include <QStringLiteral>

#include <qobject.h>

#include <algorithm>
#include <iterator>

#include "action.h"
#include "commandline.h"
#include "common.h"
#include "controller.h"
#include "hint.h"
#include "logging.h"
#include "overlay.h"

using std::copy_if;

LOGGING_CATEGORY_COLOR("tetradactyl.overlay", Qt::yellow);

namespace Tetradactyl {

// target need not be the target of the  WindowController
Overlay::Overlay(WindowController *windowController, QWidget *target,
                 bool isMain)
    : QWidget(target), controller(windowController), p_selectedHint(nullptr),
      p_statusIndicator(nullptr), p_commandLine(nullptr) {
  Q_ASSERT(controller != nullptr);
  Q_ASSERT(target != nullptr);
  // Since Overlay is not in any layout, this is needed.
  setFixedSize(2000, 2000);
  setAttribute(Qt::WA_TransparentForMouseEvents);

  // only make status indicator and command line for main overlays
  if (isMain) {
    p_statusIndicator = new QLabel(
        enumKeyToValue<ControllerMode>(windowController->controllerMode()),
        this);
    p_statusIndicator->setObjectName("overlay_status_indicator");
    p_statusIndicator->setStyleSheet(promptStylesheet);
    connect(windowController, &WindowController::modeChanged, p_statusIndicator,
            [this](ControllerMode mode) {
              p_statusIndicator->setText(enumKeyToValue<ControllerMode>(mode));
            });
    connect(windowController, &WindowController::hinted, p_statusIndicator,
            [this](HintMode mode) {
              p_statusIndicator->setText("Hint " +
                                         enumKeyToValue<HintMode>(mode));
            });

    p_commandLine = new CommandLine(this);
  }

  setLayout(new OverlayLayout(this));

  show();
}

// n.b. This does not include the "tracer" hint is displayed  for a short period
// after hinting has finished
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
  newHint->show();
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

void Overlay::popHint(HintLabel *hint) {
  Q_ASSERT(hint->parentWidget() == this);
  int idx = p_hints.indexOf(hint);
  Q_ASSERT(idx >= 0);
  p_hints.removeAt(idx);
  hint->setParent(parentWidget());
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

QDebug operator<<(QDebug debug, const Overlay *overlay) {
  QDebugStateSaver saver(debug);
  debug.nospace();
  debug << "Overlay(" << overlay->parentWidget();
  if (overlay->p_hints.length() > 0) {
    debug << " Hints: ";
    for (int i = 0; i != overlay->p_hints.length(); i++) {
      debug << qPrintable(overlay->p_hints[i]->text());
      if (i < overlay->p_hints.length() - 1)
        debug << ",";
    }
  }
  debug << ")";
  return debug;
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

OverlayLayout::OverlayLayout(Overlay *overlay)
    : QLayout(overlay), statusIndicatorItem(nullptr) {
  if (overlay->p_statusIndicator)
    statusIndicatorItem = new QWidgetItem(overlay->p_statusIndicator);
  if (overlay->p_commandLine)
    commandLineItem = new QWidgetItem(overlay->p_commandLine);
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
  QRect hostGeometry = overlay()->parentWidget()->geometry();
  for (auto item : items) {
    HintLabel *hint = qobject_cast<HintLabel *>(item->widget());
    QRect hintGeometry = QRect(hint->positionInOverlay(), hint->sizeHint());
    if (!hintGeometry.intersects(hintGeometry)) {
      logWarning << "Creating hint " << hint << "at" << hintGeometry
                 << "which doesn't intersect overlay host geometry"
                 << hostGeometry;
    }
    item->setGeometry(hintGeometry);
  }
  if (statusIndicatorItem) {
    QSize statusIndicatorSize = statusIndicatorItem->sizeHint();
    QRect statusIndicatorGeometry(
        QPoint(hostGeometry.size().width(), hostGeometry.size().height()),
        statusIndicatorSize);
    statusIndicatorGeometry.translate(-statusIndicatorSize.width(),
                                      -statusIndicatorSize.height());
    statusIndicatorItem->setGeometry(statusIndicatorGeometry);
  }

  if (commandLineItem) {
    QSize commandLineSize = commandLineItem->sizeHint();
    QRect commandLineGeometry(QPoint(0, hostGeometry.size().height()),
                              commandLineSize);
    commandLineGeometry.translate(0, -commandLineSize.height());
    commandLineItem->setGeometry(commandLineGeometry);
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

QSize OverlayLayout::sizeHint() const {
  // can't use  widget()->size() here because in Qt5 widget() isn't const
  return minimumSize();
};

QSize OverlayLayout::minimumSize() const { return QSize(2000, 2000); };

} // namespace Tetradactyl
