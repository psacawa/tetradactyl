#include <QAbstractButton>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QWindow>

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <qdebug.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qvariant.h>
#include <vector>

#include "controller.h"
#include "filter.h"
#include "hint.h"

using fmt::format;
using std::size_t;

namespace Tetradactyl {

struct ControllerSettings Controller::settings {
  .hintChars = "ASDF",
  .keymap = {.hintKey = Qt::Key_F, .cancelKey = Qt::Key_Escape}
};

bool Controller::initalized = false;
QString Controller::stylesheet = "";

Controller::Controller(QWindow *_window) : QObject(_window), window(_window) {
  assert(_window);
  filter = new KeyboardEventFilter(window, this);
  if (!initalized) {
    Controller::stylesheet = fetchStylesheet();
    qApp->setStyleSheet(Controller::stylesheet);
  }
}

Controller::~Controller() {}

void Controller::hint() {
  assert(state == State::Normal);
  qDebug() << __PRETTY_FUNCTION__;
  hintBuffer = "";
  QList<QWidget *> hintables = this->hintables();
  HintGenerator hintStringGenerator(Controller::settings.hintChars,
                                    hintables.length());
  for (auto widget : hintables) {
    string hintStr = *hintStringGenerator;
    qDebug() << "Hinting " << widget << " with " << hintStr.c_str();
    HintLabel *hint = new HintLabel(*hintStringGenerator, widget);
    hint->parentWidget()->setProperty("selected", QVariant(true));
    hint->show();
    hints.push_back(hint);
    hintStringGenerator++;
  }
  selectedHint = hints.begin();
  state = State::Hint;
}

void Controller::acceptCurrent() {}

void Controller::accept(QWidget *widget) {
  qInfo() << "Accepted " << widget;
  if (auto button = qobject_cast<QAbstractButton *>(widget)) {
    button->click();
  } else {
    qWarning() << "Don't know how to activate " << widget;
  }
  cancel();
}

void Controller::pushKey(char ch) {
  assert(state == State::Hint);
  hintBuffer += ch;
  filterHints();

  // if selectedHint hidden, set new one
  // if ((*selectedHint)->isHidden()) {
  //   selectedHint = visibleHints.begin();
  // }
}

void Controller::popKey() {
  assert(state == State::Hint);
  // pop char from hintBuffer
  hintBuffer =
      hintBuffer.substr(0, hintBuffer.size() - 1 ? hintBuffer.size() : 0);
  filterHints();
}

void Controller::filterHints() {
  vector<QWidget *> invisibleHints;
  visibleHints = {};
  std::partition_copy(hints.begin(), hints.end(),
                      std::back_inserter(visibleHints),
                      std::back_inserter(invisibleHints), [&](HintLabel *hint) {
                        return hint->text().startsWith(hintBuffer.c_str());
                      });
  for (auto widget : visibleHints) {
    widget->show();
  }
  for (auto widget : invisibleHints) {
    widget->hide();
  }
  if (visibleHints.size() == 1) {
    accept(qobject_cast<QWidget *>(visibleHints[0]->parent()));
  } else if (visibleHints.size() == 0) {
    cancel();
  }
}

void Controller::cancel() {
  assert(state == State::Hint);
  qDebug() << __PRETTY_FUNCTION__;
  for (auto hint : hints) {
    delete hint;
  }
  hints = {};
  visibleHints = {};
  state = State::Normal;
}

QList<QWidget *> Controller::hintables() {
  // QWidget *toplevelWidget =  this->window->wind
  // return QList<QWidget *>();
  QWidget *toplevelWidget = qApp->activeWindow();
  QList<QAbstractButton *> hintables =
      toplevelWidget->findChildren<QAbstractButton *>();
  // TODO 02/08/20 psacawa: dla się bez tego kopiowania?
  // return QList<QWidget *>(hintables.begin(), hintables.end());
  return QList<QWidget *>(hintables.begin(), hintables.end());
}

HintGenerator::HintGenerator(const char *_hintChars, size_t size)
    : hintChars(_hintChars), pos(0) {
  lengthOfHints = 0;
  // find the minimum lengthOfHints to be able to generate enough hint strings
  for (int numHintsGenerated = 1; numHintsGenerated < size;
       numHintsGenerated *= hintChars.size()) {
    lengthOfHints++;
  }
  // edge case
  if (lengthOfHints == 0) {
    lengthOfHints = 1;
  }
}

HintGenerator::~HintGenerator() {}

string HintGenerator::operator*() { return generateHint(); }

HintGenerator &HintGenerator::operator++() {
  pos++;
  return *this;
}

HintGenerator &HintGenerator::operator++(int _) {
  // TODO 22/07/20 psacawa: fix pre-inc
  operator++();
  return *this;
}

string HintGenerator::generateHint() {
  string ret(lengthOfHints, ' ');
  size_t tmp = pos;
  for (ssize_t idx = lengthOfHints - 1; idx >= 0; idx--) {
    ret[idx] = hintChars[tmp % hintChars.length()];
    tmp /= hintChars.length();
  }
  return ret;
}

QString fetchStylesheet() {
  QFile hintsCss(":/tetradactyl/hints.css");
  hintsCss.open(QIODevice::ReadOnly);
  QString ret = hintsCss.readAll();
  return ret;
}

} // namespace Tetradactyl
