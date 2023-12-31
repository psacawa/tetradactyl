<!-- Copyright 2023 Paweł Sacawa. All rights reserved. -->

# Notes

## Intercepting Control

In Qt C++ we have several methods of intercepting control, with varying domains of applicapility. They are listed below in decreasing order of portability/ease:

- Install `QObject` event filter.
- Accepting ShortcutOverride
- Install probe using QT's startup/object creation hooks.
- Intercept dynamic linker via `LD_PRELOAD` etc.
- Attach overlay widget to intercept mouse events.
- Install custom style to snoop on primitive element rendering.

In combination with the above, we get techniques of speculative value:

- General binary code injection/hooking techniques.
- Overwrite vtable entries.
- Dynamically subclass and replace core QT classes to override:
- `QApplication::notify`, `QWidget::event`, etc...

Whether this is worthwhile is hard to say.

## Rendering

Two basic techniques for rendering the hint where we need it to be:

- Child approach: create a `HintLabel` child widget to the target and pray that it ends up in the right place. This works quite broadly well for basic widgets `QAbstractbutton` , `QLineEdit`, etc...
- Paint approach: Hijack the rendering process in the call stack underneath `paintEvent`, most commonly in the `QStyle` subclasses. There may ultimately be a `HintLabel` widget, but it's not a child widget of the target.

The child approach is unfortunately commonly impossible for the majority of "composite" widgets which we'd like to hint: `QTabBar`, `QMenuBar`, `Q(List|Table|Tree)View`, etc. The reason that for these widgets don't have actual `QWidget` instances to represent their actionable subentities. Rather they invoke methods like `drawPrimitiveElement` and `drawControlElement` on the `QStyle` or `QStylePainter` with. the correspoind `CE_*` or `PE_*`. This

Two way to implement the paint approach:

1. Hijack `paintEvent` entirely (in a subclass?). Run the original routine and then overwrite with your own modifications.
2. Hijack `QStyle::drawPrimitiveElement` et al. and run the same logic there instead.

These both will have the hints necessarily clipped by the enclosing widget.

This suggests a _hybrid overlay approach_: like above, hijack some part of the rendering process, but only for the purpose of detecting the existence of hintable objects and their position in the global window geometry. Then render all hint labels as children of some transparent overlay-like widget which is positioned at the origin of the window space.

This enables us to avoid clipping the hints, but it also provdes in one place the logic necessary to layout hint labels. This will enable us to write code which solves the overlapping hints problem. In general, an overlay is necessary if we want to e.g. have a timed delay of widgets.

The main downsides are the downsides of overlay widgets: the order and execution of painting code is tightly coupled to Tetradactyl's correct operation. How to ensure that painting always happens when it needs to? Since the overlay needs to painted last, that means it will be on top. Then it will eat mouse events... A widget attribute `Qt::WA_TransparentForMouseEvents` saves the day.

### C++

Qt uses C++ with the MOC code genertor. To use the symbols in the same way as C, we have to deal with issues of ABI on all the supported platforms. Namely:

1. Symbol mangling obscures symbols. This is not even necessarily portable between compilers. To get access to intercepted symbols via `dlsym` et al., we need _mangling_ support at runtime. Where to get it? Even within a given compiler, the mangling may be dependent on e.g. c++ standard used in program and/or compiler flags.

2. Issues with redefining symbols declared in (possibly private) headers in a semi-automatic manner may be more severe now due to namespaces etc. Need a code generator

3. C++ has more complex semantics, and many function calls are implicit (constructors, destructors). The logic to determine them is in the compiler. Also their signatures may actually differ from their ABI, so it's not a matter of just tranmitting the received arguments.

This is a big challenge to any Tetradactyl backend targetting a C++ framework.

# Particular Widgets/Events

## Context Menu

Three (main) modes of using context menus in Qt:

- `contextMenuPolicy == Qt::ActionsContextMenu`, default menu with the `QWidget::actions`. This is fine.
- `contextMenuPolicy == Qt::CustomContextMenu`, sends a signal. It's fine.
- `contextMenuPolicy == Qt::DefaultContextMenu`, `QApplication::notify` will call `contextMenuEvent` virtual method, which is hopefully overwritten.

Short of inspecting the vtable, there is really no simple way to detect the third (default) case. `notify` will always return `true`. Event filters act before we reach the event handler. Overview of prominent codebases seems to indicate usage is roughly evenly split between `CustomContextMenu` and `DefaultContextMenu`. It may be necessary to trigger the `ContextMenuEvent` just to probe whether any menu was created. But that's stupid...

## Parent/NativeParent Widgets

Given the decision to make dialogs have their own window controllers, special attention is given to routines like `Controller::findControllerForWidget`.

The `nativeParentWidget` of `QMenu` is the controlling window, even in the case of a submenu. In the case of `QCombobox`. It launches a `QComboboxPrivateContainer` which is a native widget, and is the parent of the `QCombobox` list view. The `nativeParentWidget` of the `QComboboxPrivateContainer` is the controlling window.

## Concurrency

There is no control on when the client manipulates the application widgets. We must hope that he does so only from the UI thread, e.g. deferring signal handlers connected to across thread boundaries. If the client does e.g. use `Qt::DirectConnection` across threads, or constructs a `QWidget` subclass outside of main thread, we should try and find this out. Under the assumption that he doesn't, this means that we have control of the widget tree upon each event loop reentry that finds its way into our control flow. The operations from that point should be done transactionally: if we determine they can't be accomplished, abort with an error (instead of trying them and segfaulting). Therefore the relevant code sections must be guarded by asserts that will terminate the operation upon failure.

## ABI

Qt outright refuses to interoperate with library sibling versions other than it's own. Therefore if we have e.g. vlc which uses `libQt5Widgets` (in `qtbase`) and `libQt5Widgets` (not), then when the application is run via Tetradactyl the libs loaded can be respectively `/usr/local/Qt-5.15.10/lib/libQt5Widgets.so.5.15.10` and `/usr/lib/x86_64-linux-gnu/libQt5Svg.so.5.15.6`. This means we must in princip distribute the entirety of qt super module to clients to ensure interoperation.
