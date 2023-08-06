#pragma once

#include <QDebug>
#include <QWidget>
#include <QWindow>

#include <vector>

#include "filter.h"
#include "hint.h"

using std::size_t;
using std::string;
using std::vector;

namespace Tetradactyl {

struct ControllerKeymap {
  Qt::Key hintKey;
  Qt::Key cancelKey;
};

struct ControllerSettings {
  const char *hintChars;
  ControllerKeymap keymap;
};

class Controller : public QObject {
  Q_OBJECT

public:
  enum State { Normal, Hint };

  Controller(QWindow *_window);
  virtual ~Controller();

  void hint();
  void acceptCurrent();
  QList<QWidget *> hintables();
  void cancel();
  void pushKey(char ch);
  void popKey();

  State state = State::Normal;
  QWindow *window;
  KeyboardEventFilter *filter;
  vector<HintLabel *> hints;
  vector<HintLabel *> visibleHints;
  // Currently "active" hint. <enter> will accept it. May be invalidated when
  // hintBuffer gets input
  // TODO 02/08/20 psacawa: custom iterator that only touches visible widgets
  vector<HintLabel *>::iterator selectedHint;
  string hintBuffer;
  static ControllerSettings settings;
  static QString stylesheet;
  static bool initalized;

private:
  void filterHints();
  void accept(QWidget *widget);
};

int pow(int b, unsigned e);

class HintGenerator {

public:
  HintGenerator(const char *_hintChars, size_t size);
  virtual ~HintGenerator();

  string operator*();
  HintGenerator &operator++();
  HintGenerator &operator++(int _);

private:
  string generateHint();

  string hintChars;
  size_t lengthOfHints;
  size_t pos;
};

QString fetchStylesheet();

} // namespace Tetradactyl
