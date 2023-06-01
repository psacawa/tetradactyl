## Event-base programming

Two keys entities for event-based programming in GTK/GObject systems: signals, events. 

Signals are defined on objects and are inherited. They run synchronously. They run in order of attachment unless some flag was given to `g_signal_connect`. They may receive params from emitter. A constant "user data" may be passed by client connecting signal.

Events represent input from the user/window system. These propagate through the widget tree to their target in a "capture phase", thenperform the "target phase" at their target, then do "bubble phase" upwards again. This is like DOM. At each widget on the way, their may be an event controller, which emits the "key-pressed", "clicked" etc. signals in response to the event (see `*_handle_events` stack). Any callback attached can stop propagation via return value.

`How target of events relates to focus, etc.?`

## Shortcuts

Types: Accelerators for global actions, mnemonics for things with labels, `EventControllerKey`s attached to particular widgets activate in target phase. `ShortcutController` has a list of actions on `priv->shortcuts`. Each `GtkShortcut` has  `GtkShortcutAction` (not a `GAction`) and `GtkShortcutTrigger`.
 
## Details

### Controllers

On windows, we have 11 controllers. From `gtk_window_init` d2d, motion, unnamed key controller, legacy, menubar-accels.  Two more come from `ShortcutManager` interface: capture+bubble. From `gtk_window_constructed` we have click-gesture controller, from  `gtk_window_set_application` we have `gtk-application-shortcuts`. `GtkWidget` has `gtk-widget-class-shortcuts`
