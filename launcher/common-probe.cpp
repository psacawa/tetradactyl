
#include "common.h"
#include "libnames.h"

using std::vector;
using Tetradactyl::BackendData;
using Tetradactyl::WidgetBackend;

vector<BackendData> backends = {
    {WidgetBackend::Gtk3, GTK3_LIB, GTK3_TETRADACTYL_LIB},
    {WidgetBackend::Gtk4, GTK4_LIB, GTK4_TETRADACTYL_LIB},
    {WidgetBackend::Qt5,QT5_LIB, QT5_TETRADACTYL_LIB},
    {WidgetBackend::Qt6,QT6_LIB, QT6_TETRADACTYL_LIB}};
