// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QAbstractButton>
#include <QDebug>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QWidget>
#include <QWindow>

#include <map>
#include <vector>

#include "filter.h"
#include "hint.h"
#include "overlay.h"

using std::size_t;
using std::string;
using std::vector;

namespace Tetradactyl {
Q_NAMESPACE

class WindowController;

struct ControllerKeymap {
  QKeySequence activate;
  QKeySequence cancel;
  QKeySequence edit;
  QKeySequence focus;
  QKeySequence yank;
  QKeySequence activateMenu;
  QKeySequence activateContext;
  QKeySequence upScroll;
  QKeySequence downScroll;
};

struct ControllerSettings {
  const char *hintChars;
  bool passthroughKeyboardInput;
  bool passthroughKeyboardInputDelay;
  ControllerKeymap keymap;
};

enum HintMode { Activatable, Editable, Yankable, Focusable, Contextable };
Q_ENUM_NS(HintMode);

// Manages global Tetradactyl state of application.
class Controller : public QObject {
  Q_OBJECT

public:
  Controller(QWidget *parent = nullptr);
  virtual ~Controller();

  static ControllerSettings settings;
  static QString stylesheet;
  static Controller *instance();

public slots:
  static void createController();
  void routeObject(QObject *obj);

private:
  bool eventFilter(QObject *obj, QEvent *ev);
  void tryAttachToWindow(QWidget *widget);
  void attachToExistingWindows();
  static const std::map<HintMode, vector<const QMetaObject *>>
      hintableMetaObjects;
  QList<WindowController *> windowControllers;

  static Controller *self;
};

inline Controller *Controller::instance() { return Controller::self; }

// Manages Tetradactyl state for each toplevel "main" widgets. This means
// Widgets that got shown, including diaglogs, but not WindowType::Popup
// widgets like QMenu
class WindowController : public QObject {
  Q_OBJECT
public:
  enum ControllerMode { Normal, Hint, Input };
  Q_ENUM(ControllerMode);

  WindowController(QWidget *target, QObject *_target);
  virtual ~WindowController() {}

  QList<QWidget *> getHintables(HintMode mode);
  ControllerMode mode = ControllerMode::Normal;
  HintMode currentHintMode;

  QWidget *host();

public slots:

  void hint(HintMode mode = Activatable);
  void acceptCurrent();
  void cancel();
  void pushKey(char ch);
  void popKey();

private:
  bool eventFilter(QObject *obj, QEvent *ev);
  void accept(QWidget *widget);
  void filterHints();
  void initializeShortcuts();
  void initializeOverlays();
  void tryAttachController(QWidget *widget);

  Controller *controller;
  QWidget *target;
  QList<Overlay *> overlays;
  vector<HintLabel *> hints;
  vector<HintLabel *> visibleHints;
  // Currently "active" hint. <enter> will accept it. May be invalidated when
  // hintBuffer gets input
  // TODO 02/08/20 psacawa: custom iterator that only touches visible widgets
  vector<HintLabel *>::iterator selectedHint;
  string hintBuffer;
};

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
