## Event-base programming

Two keys entities for event-based programming in GTK/GObject systems: signals, events.

Signals are defined on objects and are inherited. They run synchronously. They run in order of attachment unless some flag was given to `g_signal_connect`. They may receive parameters from emitter. A constant "user data" may be passed by client connecting signal.

Events represent input from the user/window system. These propagate through the widget tree to their target in a "capture phase", then perform the "target phase" at their target, then do "bubble phase" upwards again. This is like DOM. At each widget on the way, their may be an event controller, which emits the "key-pressed", "clicked" etc. signals in response to the event (see `*_handle_events` stack). Any callback attached can stop propagation via return value.

`How target of events relates to focus, etc.?`

### Controllers

Several subclasses of `EventController`. They convert GDK events to signals they emit. Some subclasses to look at are `GestureClick` and `EventControllerKey` and `ShortcutController`.

`GestureClick` emits _pressed_ and _released_ signals.
`EventControllerKey` emits _key-pressed_, _key-released_, _im-update_ , and _modifiers_.
`ShortcutController` is different: it it a list of shortcuts that can be activated by keyboard. `GtkWidget`. Each widget has one called "gtk-widget-class-shortcuts" which has global shortcuts. Added via `gtk_shortcut_controller_new_for_model`. This appears to be unique per class. Constructed in `gtk_widget_init` via

```
gtk_shortcut_controller_new_for_model (G_LIST_MODEL (GTK_WIDGET_CLASS (g_class)->priv->shortcuts));
```

On windows, we have 11 controllers. From `gtk_window_init` d2d, motion, unnamed key controller, legacy, menubar-accels. Two more come from `ShortcutManager` interface: capture+bubble. From `gtk_window_constructed` we have click-gesture controller, from `gtk_window_set_application` we have `gtk-application-shortcuts`. `GtkWidget` has `gtk-widget-class-shortcuts`

## Shortcuts/Actions

### Actions

From gio, we have `GAction`, `GActionGroup`, `GActionMap`, all interfaces. `GAction` has properties name, parameter and state. `GActionGroup` has methods to activate action by name, and signals for added/removed action. `GActionMap` has methods to add/remove actions in a structured way. It consists of `GActionEntry`s, which are a user-facing struct used for loading actions into maps (see gtk demos).

### Shortcuts

Types: Accelerators for global actions, mnemonics for things with labels, `EventControllerKey`s attached to particular widgets activate in target phase. `ShortcutController` has a list of actions on `priv->shortcuts`. Each `GtkShortcut` has `GtkShortcutAction` (not a `GAction`) and `GtkShortcutTrigger`.

`GtkShortcutAction`s are different from actions. There are subclasses for signals,activation,mnemonics. For triggers, there are subclasses for keyvals, mnemonics.

## Example What happens for a button?

A button is in a window, which is controlled by an application.

### In Case of a Click

The button has a GestureClick controller in capture phase, which responds to `GDK_BUTTON_PRESS` events. The button has "clicked" and "activate" signals, while the `GestureClick` has "pressed" and "released". On button init, the "released" signal of gesture is connected to `click_released_cb`, which invokes `gtk_button_do_release`, which emits "clicked" signal, to which the application is hopefully connected.

### In Case of keyboard activation.

The button has a `EventControllerKey` which releases signals "key-pressed" and "key-released". On `GDK_KEY_RELEASE` the controller releases the `key-released` signal, which is connected `key_controller_key_released_cb` on the button. This calls `gtk_button_finish_activate`, which emits the "clicked" signal. Access to this is guarded by `priv->action_timeout`, which was  set in `gtk_real_button_activate` (how?)

This  `action_timeout` has set the `ShortcutController` on the `GDK_KEY_PRESS` event. THis causes the `ShortcutController` to emit the "activate" action on the widget. See  `gtk_shortcut_action_activate` and below.

The button also has the `ShortcutController` "gtk-widget-class-shortcuts" , which responds to `GDK_KEY_RELEASE` events associated with its shortcuts, which have `SignalAction`s with trigger `space` and `enter`. The actions are the `activate`. This appears to play a secondary role (both a bubble, and it has lower priority as a controller)

TODO 04/06/20 psacawa: Elaborate on meaning of `ShortcutController` and `GtkActionable` interface on button.

If actionable name is set on button, then `gtk_real_button_clicked` will be attached uniquely to "clicked" _after_ the other handlers

## Actionable

Interface implemented on button. Implemented on buttons. Has name and target. Implemented via private action helper, which has a muxer. Older than `GtkShortcut`, with which it has some similarity (2012 v. 2018). The point seems to be that the muxer can recursively looks for actions defined elsewhere, thus `win.*` or `default.*` can be attached. The muxer has a `GActionGroup`-like interface.


## GObject

Suppose Derived < Base. Each subclass of Base has its own instance of `BaseClass`, and `base_class_init` is run alongside `derived_class_init`. ...
