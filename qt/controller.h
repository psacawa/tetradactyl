// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once

#include <QAbstractButton>
#include <QApplication>
#include <QDebug>
#include <QKeySequence>
#include <QList>
#include <QMap>
#include <QPointer>
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
  // TODO 22/09/20 psacawa: finish this
  // None,
  Activatable,
  Editable,
  Yankable,
  Focusable,
  Contextable,
  Menuable

};
Q_ENUM_NS(HintMode);

enum ControllerMode { Normal, Hint, Input, Ignore };
Q_ENUM_NS(ControllerMode);

QWidget *getToplevelWidgetForWindow(QWindow *win);

// Manages global Tetradactyl state of application.
class Controller : public QObject {
  Q_OBJECT

public:
  Controller();
  virtual ~Controller();

  Q_PROPERTY(QList<WindowController *> windows READ windows);
  Q_PROPERTY(QString stylesheet MEMBER stylesheet);
  // this can be uncommented only when the classed is built up into a proper
  // QObject Q_PROPERTY(ControllerSettings *settings MEMBER settings);

  static const Controller *instance();
  const QList<WindowController *> &windows() const;

  const ControllerSettings *controllerSettings() {
    return &Controller::settings;
  }

  static ControllerSettings settings;
  static QString stylesheet;

public slots:
  static void createController();
  void routeNewlyCreatedObject(QObject *obj);
  void resetModeAfterFocusChange(QWidget *old, QWidget *now);
  void resetModeAfterFocusWindowChanged(QWindow *focusWindow);

private:
  bool eventFilter(QObject *obj, QEvent *ev);
  void attachControllerToWindow(QWidget *widget);
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
  Q_PROPERTY(Controller *controller MEMBER controller);
  Q_PROPERTY(QWidget *target READ target);
  Q_PROPERTY(ControllerMode controllerMode READ controllerMode WRITE
                 setControllerMode);
  Q_PROPERTY(BaseAction *currentAction READ currentAction);
  Q_PROPERTY(
      HintMode currentHintMode READ currentHintMode WRITE setCurrentHintMode);
  Q_PROPERTY(QList<QPointer<Overlay>> overlays READ overlays);

  WindowController(QWidget *target, QObject *parent);
  virtual ~WindowController();

  Overlay *findOverlayForWidget(QWidget *);
  const QList<QPointer<Overlay>> overlays();
  Overlay *mainOverlay();
  Overlay *activeOverlay();

  QList<QWidget *> getHintables(HintMode hintMode);

  HintMode currentHintMode();
  void setCurrentHintMode(HintMode mode);
  ControllerMode controllerMode();
  void setControllerMode(ControllerMode mode);

  QWidget *target();
  bool earlyKeyEventFilter(QKeyEvent *ev);
  void addOverlay(QWidget *target);
  void removeOverlay(Overlay *overlay, bool fromSignal = false);
  bool isActing();
  BaseAction *currentAction() { return p_currentAction; }

public slots:

  void hint(HintMode mode = Activatable);
  void acceptCurrent();
  void cancel();
  void escapeInput();
  void pushKey(char ch);
  void popKey();

signals:
  void modeChanged(ControllerMode mode);
  void hintModeChanged(HintMode mode);

  void hinted(HintMode hintMode);
  // Fired when a single stage of hinting finished with a hint accepted by the
  // user. In the case where we go into menus, the target widget is the widget
  // controlling access to the menu, i.e the QMenuBar.
  void accepted(HintMode hintMode, QWidget *target,
                QPoint positionInWidget = QPoint(0, 0));
  // in the case a menu has been accepted, the target below will be the
  // corresponding menuAction.
  // TODO 21/09/20 psacawa: implement the hinting of this
  void acceptedAction(HintMode hintMode, QAction *action);
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

  HintMode p_currentHintMode;
  ControllerMode p_controllerMode = ControllerMode::Normal;
  BaseAction *p_currentAction;
  Controller *controller;
  QWidget *p_target;
  QList<QPointer<Overlay>> p_overlays;
  QList<QPointer<QShortcut>> shortcuts;
  // Currently "active" hint. <enter> will accept it. May be invalidated when
  // hintBuffer gets input
  // TODO 02/08/20 psacawa: custom iterator that only touches visible widgets
  QString hintBuffer;

  friend QDebug operator<<(QDebug debug, const WindowController *controller);
};

inline const QList<QPointer<Overlay>> WindowController::overlays() {
  return p_overlays;
}

inline bool WindowController::isActing() { return p_currentAction != nullptr; }

inline HintMode WindowController::currentHintMode() {
  return p_currentHintMode;
}
inline void WindowController::setCurrentHintMode(HintMode mode) {
  bool changed = mode != p_currentHintMode;
  p_currentHintMode = mode;
  if (changed) {
    emit hintModeChanged(mode);
  }
}
inline ControllerMode WindowController::controllerMode() {
  return p_controllerMode;
}
inline void WindowController::setControllerMode(ControllerMode mode) {
  bool changed = mode != p_controllerMode;
  p_controllerMode = mode;
  for (auto sc : shortcuts)
    sc->setEnabled(mode == Normal);

  if (!changed)
    return;

  if (mode != Hint) {
    cleanupHints();
  }

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

bool inputModeWhenWidgetFocussed(QWidget *w);

} // namespace Tetradactyl
