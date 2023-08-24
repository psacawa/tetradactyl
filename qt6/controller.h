#pragma once

#include <QAbstractButton>
#include <QDebug>
#include <QKeySequence>
#include <QMap>
#include <QWidget>
#include <QWindow>

#include <map>
#include <vector>

#include "filter.h"
#include "hint.h"

using std::size_t;
using std::string;
using std::vector;

namespace Tetradactyl {

struct ControllerKeymap {
  QKeySequence activate;
  QKeySequence cancel;
  QKeySequence edit;
  QKeySequence focus;
  QKeySequence yank;
  QKeySequence upScroll;
  QKeySequence downScroll;
};

struct ControllerSettings {
  const char *hintChars;
  bool passthroughKeyboardInput;
  bool passthroughKeyboardInputDelay;
  ControllerKeymap keymap;
};

class Controller : public QObject {
  Q_OBJECT

public:
  enum ControllerMode { Normal, Hint, Input };
  Q_ENUM(ControllerMode);
  enum HintMode { Activatable, Editable, Yankable, Focusable, Contextable };
  Q_ENUM(HintMode);

  Controller(QWindow *_window);
  virtual ~Controller();

  QList<QWidget *> getHintables(HintMode mode);
  ControllerMode mode = ControllerMode::Normal;
  HintMode currentHintMode;
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

public slots:

  void hint(HintMode mode = Activatable);
  void acceptCurrent();
  void cancel();
  void pushKey(char ch);
  void popKey();

private:
  void filterHints();
  QWidget *myToplevelWidget();
  void accept(QWidget *widget);

  static const std::map<HintMode, vector<const QMetaObject *>>
      hintableMetaObjects;

  friend class KeyboardEventFilter;
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
