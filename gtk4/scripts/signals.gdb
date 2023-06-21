# a janky ptrace-based gojbect signal  tracing util, just to show it's possible
# run as:
# gdb -x scripts/signals.gdb build/gtk-demo

dashboard -enabled off

add-symbol-file /home/psacawa/lib/libgobject-2.0.so.0

break -f g_signal_emit 
commands 1 print_signal
print g_signal_name(signal_id)
continue
end
