#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QWindow>

#include <QtCore/private/qhooks_p.h>

namespace Tetradactyl {

// A singleton binding the Qt hooks to the information necessary for probing
// created QObjects for QWindow etc. The class may well by superfluous.
class Probe {
public:
  Probe();

  // hooks must be static for ABI reasons (no hidden "this" pointer param)
  static void addQObjectCallback(QObject *obj);
  static void removeQObjectCallback(QObject *obj);
  static void startupCallback();

  static void afterAppInitialization();
  static void attachControllerToWindows();

  static Probe *instance();

private:
  QList<QObject *> createdObjects;
  QTimer timer;
  static Probe *p_instance;
};
} // namespace Tetradactyl
