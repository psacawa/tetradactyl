// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QAbstractButton>
#include <QApplication>
#include <QDebug>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QShortcut>
#include <QWidget>
#include <QWindow>

#include <map>
#include <vector>

using std::size_t;
using std::string;
using std::vector;

// class HintLabel;
namespace Tetradactyl {
Q_NAMESPACE

class HintLabel;
class Overlay;
class BaseAction;
class QWidgetActionProxy;

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
  bool autoAcceptUniqueHint;
  bool highlightAcceptedHint;
  int highlightAcceptedHintMs;
  bool passthroughKeyboardInput;
  ControllerKeymap keymap;
};

enum HintMode {
  Activatable,
  Editable,
  Yankable,
  Focusable,
  Contextable,
  Menuable
};
Q_ENUM_NS(HintMode);

// Manages global Tetradactyl state of application.
class Controller : public QObject {
  Q_OBJECT

public:
  Controller();
  virtual ~Controller();

  static const Controller *instance();
  const QList<WindowController *> &windows() const;

  static ControllerSettings settings;
  static QString stylesheet;

public slots:
  static void createController();
  void routeNewlyCreatedObject(QObject *obj);

private:
  bool eventFilter(QObject *obj, QEvent *ev);
  void tryAttachToWindow(QWidget *widget);
  void attachToExistingWindows();
  WindowController *findControllerForWidget(QWidget *);
  static const std::map<HintMode, vector<const QMetaObject *>>
      hintableMetaObjects;
  QList<WindowController *> windowControllers;

  static Controller *self;

  friend QDebug operator<<(QDebug debug, const Controller *controller);
};

inline const Controller *Controller::instance() { return Controller::self; }
inline const QList<WindowController *> &Controller::windows() const {
  return windowControllers;
}

// Manages Tetradactyl state for each toplevel "main" widgets. This means
// Widgets that got shown, including diaglogs, but not WindowType::Popup
// widgets like QMenu
class WindowController : public QObject {
  Q_OBJECT
public:
  enum ControllerMode { Normal, Hint, Input, Ignore };
  Q_ENUM(ControllerMode);

  WindowController(QWidget *target, QObject *parent);
  virtual ~WindowController();

  Overlay *findOverlayForWidget(QWidget *);
  const QList<Overlay *> &overlays();
  Overlay *mainOverlay();
  Overlay *activeOverlay();

  QList<QWidget *> getHintables(HintMode hintMode);
  HintMode currentHintMode;

  ControllerMode controllerMode();
  void setControllerMode(ControllerMode mode);

  QWidget *target();
  bool earlyKeyEventFilter(QKeyEvent *ev);
  void addOverlay(QWidget *target);
  void removeOverlay(Overlay *overlay, bool fromSignal = false);
  bool isActing();

public slots:

  void hint(HintMode mode = Activatable);
  void acceptCurrent();
  void cancel();
  void pushKey(char ch);
  void popKey();

signals:
  void modeChanged(ControllerMode mode);
  void hinted(HintMode hintMode);
  // Fired when a single stage of hinting finished with a hint accepted by the
  // user
  void accepted(HintMode hintMode, QWidget *widget,
                QPoint positionInWidget = QPoint(0, 0));
  // Fired when a hinting is finished with a cancellation with <esc> by user
  void cancelled(HintMode hintMode);
  // Fired when a hinting finished entirely, successfully or not
  void hintingFinished(bool accepted);

private:
  void cleanupHints();
  void cleanupAction();
  bool eventFilter(QObject *obj, QEvent *ev);
  void accept(QWidgetActionProxy *widgetProxy);
  void filterHints();
  void initializeShortcuts();
  void initializeOverlays();
  void tryAttachController(QWidget *widget);

  ControllerMode p_controllerMode = ControllerMode::Normal;
  BaseAction *currentAction;
  Controller *controller;
  QWidget *p_target;
  QList<Overlay *> p_overlays;
  QList<QShortcut *> shortcuts;
  // Currently "active" hint. <enter> will accept it. May be invalidated when
  // hintBuffer gets input
  // TODO 02/08/20 psacawa: custom iterator that only touches visible widgets
  QString hintBuffer;

  friend QDebug operator<<(QDebug debug, const WindowController *controller);
};

inline const QList<Overlay *> &WindowController::overlays() {
  return p_overlays;
}
// TODO 10/09/20 psacawa: rozwiń
inline Overlay *WindowController::activeOverlay() {
  QWidget *activeWidget = qApp->activePopupWidget();
  if (!activeWidget)
    activeWidget = qApp->focusWidget();
  return findOverlayForWidget(activeWidget);
}
inline bool WindowController::isActing() { return currentAction != nullptr; }
inline WindowController::ControllerMode WindowController::controllerMode() {
  return p_controllerMode;
}
inline void WindowController::setControllerMode(ControllerMode mode) {
  bool changed = mode != p_controllerMode;
  p_controllerMode = mode;
  if (changed)
    emit modeChanged(mode);
}

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
