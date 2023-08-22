#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QKeySequence>
#include <QLineEdit>
#include <QList>
#include <QLoggingCategory>
#include <QShortcut>
#include <QTextEdit>
#include <QWindow>

// #include <QtWidgets/private>

#include <fmt/core.h>

#include <algorithm>
#include <iterator>
#include <qassert.h>
#include <qdebug.h>
#include <qglobal.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qt6/QtCore/qmap.h>
#include <qvariant.h>
#include <vector>

#include "controller.h"
#include "filter.h"
#include "hint.h"

#define THIS_LOG tetradactylController
Q_LOGGING_CATEGORY(tetradactylController, "tetradactyl.controller");

using std::size_t;

namespace Tetradactyl {

const std::map<Controller::HintMode, vector<const QMetaObject *>>
    Controller::hintableMetaObjects = {
        {Activatable, {&QAbstractButton::staticMetaObject}},
        {Editable,
         {&QLineEdit::staticMetaObject, &QTextEdit::staticMetaObject}}};

struct ControllerSettings Controller::settings {
  .hintChars = "ASDFJKL",
  .keymap = {.hintKey = Qt::Key_F, .cancelKey = Qt::Key_Escape}
};

bool Controller::initalized = false;
QString Controller::stylesheet = "";

Controller::Controller(QWindow *_window) : QObject(_window), window(_window) {
  Q_ASSERT(_window);
  filter = new KeyboardEventFilter(window, this);
  if (!initalized) {
    Controller::stylesheet = fetchStylesheet();
    qApp->setStyleSheet(Controller::stylesheet);
  }
}

Controller::~Controller() {}

void Controller::hint(HintMode hintMode) {
  if (!(mode == ControllerMode::Normal)) {
    qCInfo(THIS_LOG) << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  qCDebug(THIS_LOG) << __PRETTY_FUNCTION__;
  hintBuffer = "";
  QList<QWidget *> hintables = this->getHintables(hintMode);
  HintGenerator hintStringGenerator(Controller::settings.hintChars,
                                    hintables.length());
  for (auto widget : hintables) {
    string hintStr = *hintStringGenerator;
    qCDebug(THIS_LOG) << "Hinting " << widget << " with " << hintStr.c_str();
    HintLabel *hint = new HintLabel(*hintStringGenerator, widget);
    hint->parentWidget()->setProperty("selected", QVariant(true));
    hint->show();
    hints.push_back(hint);
    hintStringGenerator++;
  }
  selectedHint = hints.begin();
  mode = ControllerMode::Hint;
  currentHintMode = hintMode;
}

void Controller::acceptCurrent() {}

void Controller::accept(QWidget *widget) {
  if (!(mode == ControllerMode::Hint)) {
    qCInfo(THIS_LOG) << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  qCInfo(THIS_LOG) << "Accepted " << widget << "in" << currentHintMode;
  switch (currentHintMode) {
  case Activatable: {
    if (auto button = qobject_cast<QAbstractButton *>(widget)) {
      button->click();
    } else {
      qCWarning(THIS_LOG) << "Don't know how to activate " << widget << "in"
                          << currentHintMode;
    }
    break;
  }
  case Editable: {
    widget->setFocus();
    /*
     * if (const auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
     * } else {
     *   else_block
     * }
     */
    break;
  }
  case Yankable:
    qCWarning(THIS_LOG) << "unimplemented";
    break;
  default:
    Q_UNREACHABLE();
    break;
  }
  cancel();
}

void Controller::pushKey(char ch) {
  if (!(mode == ControllerMode::Hint)) {
    qCInfo(THIS_LOG) << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  hintBuffer += ch;
  filterHints();

  // if selectedHint hidden, set new one
  // if ((*selectedHint)->isHidden()) {
  //   selectedHint = visibleHints.begin();
  // }
}

void Controller::popKey() {
  if (!(mode == ControllerMode::Hint)) {
    qCInfo(THIS_LOG) << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  // pop char from hintBuffer
  hintBuffer =
      hintBuffer.substr(0, hintBuffer.size() - 1 ? hintBuffer.size() : 0);
  filterHints();
}

QWidget *Controller::myToplevelWidget() {
  for (auto toplevel : qApp->topLevelWidgets()) {
    if (toplevel->windowHandle() == window) {
      return toplevel;
    }
  }
  Q_UNREACHABLE();
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
  if (!(mode == ControllerMode::Hint)) {
    qCInfo(THIS_LOG) << __PRETTY_FUNCTION__ << "from" << mode;
    return;
  }
  qCDebug(THIS_LOG) << __PRETTY_FUNCTION__;
  for (auto hint : hints) {
    delete hint;
  }
  hints = {};
  visibleHints = {};
  mode = ControllerMode::Normal;
}

// For now the  hinting proceed only by type: QObjects whose meta-object are
// right get hinted. However, to support clients with more complex handling of
// events, etc., we will want a more dynamic approach.
QList<QWidget *> Controller::getHintables(HintMode hintMode) {
  QWidget *toplevelWidget = myToplevelWidget();
  QList<QWidget *> descendants = toplevelWidget->findChildren<QWidget *>();
  vector<QWidget *> hintables;
  vector<const QMetaObject *> metaObjects;
  try {
    metaObjects = hintableMetaObjects.at(hintMode);
  } catch (std::out_of_range) {
    qCWarning(THIS_LOG) << "unsupported" << hintMode;
    return {};
  }

  std::copy_if(descendants.begin(), descendants.end(),
               std::back_inserter(hintables), [=](QWidget *obj) {
                 for (auto mo : metaObjects) {
                   if (mo->cast(obj) != nullptr) {
                     return true;
                   }
                 }
                 return false;
               });

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
