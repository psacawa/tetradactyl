// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QObject>

#include "common.h"

namespace Tetradactyl {

class BaseApp : public QObject {
  Q_OBJECT
public:
  // for short display, e.g. "Anki"
  Q_PROPERTY(QString name READ name);
  Q_PROPERTY(QString absolutePath READ absolutePath);
  Q_PROPERTY(WidgetBackend backend READ backend);

  BaseApp(WidgetBackend backend = WidgetBackend::Unknown)
      : p_backend(backend) {}
  virtual ~BaseApp() {}

  virtual QString name() = 0;
  virtual QString absolutePath() = 0;
  void probe();
  WidgetBackend backend();

private:
  WidgetBackend p_backend;
};

inline WidgetBackend BaseApp::backend() { return p_backend; }

class ExecutableFileApp : public BaseApp {
  Q_OBJECT
public:
  ExecutableFileApp(QString path);
  virtual ~ExecutableFileApp() {}

  virtual QString name();
  virtual QString absolutePath();

  static ExecutableFileApp fromFile(QString path);

private:
  QString p_path;
};

inline QString ExecutableFileApp::absolutePath() { return p_path; }

class XdgDesktopApp : public BaseApp {
  Q_OBJECT
public:
  Q_PROPERTY(QString commandLine READ commandLine);
  Q_PROPERTY(QString desktopPath READ desktopPath);
  Q_PROPERTY(QString desktopId READ desktopId);

  virtual ~XdgDesktopApp() {}

  virtual QString name();
  virtual QString absolutePath();
  virtual QString commandLine();
  QString desktopPath();
  QString desktopId();

  static XdgDesktopApp fromAppId(QString appId);
  static XdgDesktopApp *fromDesktopFile(QString desktopFilePath);

private:
  XdgDesktopApp() {}

private:
  QString p_id;
  QString p_desktopFilePath;
  // base name of program, e.g. "anki"
  QString p_executable;
  QString p_name;
  QString p_commandLine;
  QString p_description;
};

inline QString XdgDesktopApp::name() { return p_name; }
inline QString XdgDesktopApp::commandLine() { return p_commandLine; }
inline QString XdgDesktopApp::desktopPath() { return p_desktopFilePath; }
inline QString XdgDesktopApp::desktopId() { return p_id; }

} // namespace Tetradactyl
