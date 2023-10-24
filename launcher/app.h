// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QIcon>
#include <QJsonObject>
#include <QObject>

#include <optional>

#include "common.h"

using std::nullopt;
using std::optional;

namespace Tetradactyl {

class AbstractApp : public QObject {
  Q_OBJECT
public:
  // for short display, e.g. "Anki"
  Q_PROPERTY(QString name READ name);
  // for equality comparison mainly (.desktop for XdgDesktopApp)
  Q_PROPERTY(QString absolutePath READ absolutePath);
  Q_PROPERTY(WidgetBackend backend READ backend WRITE setBackend);
  // Q_PROPERTY(QIcon icon READ getIcon);

  AbstractApp(WidgetBackend backend = WidgetBackend::Unknown)
      : p_backend(backend) {}
  virtual ~AbstractApp() {}

  virtual QString name() const = 0;
  virtual QString absolutePath() const = 0;
  virtual QIcon getIcon() const = 0;
  virtual void launch() const = 0;
  void probe();
  WidgetBackend backend() const;
  void setBackend(WidgetBackend backend);

  virtual QJsonObject toJson() const = 0;
  static AbstractApp *fromJson(QJsonObject obj);

  bool operator==(const AbstractApp &other) {
    return QFileInfo(absolutePath()).canonicalFilePath() ==
           QFileInfo(other.absolutePath()).canonicalFilePath();
  }

protected:
  WidgetBackend p_backend;
};

inline WidgetBackend AbstractApp::backend() const { return p_backend; }
inline void AbstractApp::setBackend(WidgetBackend backend) {
  p_backend = backend;
}

class ExecutableFileApp : public AbstractApp {
  Q_OBJECT
public:
  ExecutableFileApp(QString path, optional<WidgetBackend> backend = nullopt);
  virtual ~ExecutableFileApp() {}

  virtual QString name() const override;
  virtual QString absolutePath() const override;
  virtual QIcon getIcon() const override;
  virtual void launch() const override;
  virtual QJsonObject toJson() const override;
  static ExecutableFileApp *fromJson(QJsonObject obj);

private:
  QString p_path;
};

inline QString ExecutableFileApp::absolutePath() const { return p_path; }

class XdgDesktopApp : public AbstractApp {
  Q_OBJECT
public:
  Q_PROPERTY(QString commandLine READ commandLine);
  Q_PROPERTY(QString desktopPath READ desktopPath);
  Q_PROPERTY(QString desktopId READ desktopId);

  virtual ~XdgDesktopApp() {}

  virtual QString name() const override;
  virtual QString absolutePath() const override;
  virtual QIcon getIcon() const override;
  virtual void launch() const override;

  virtual QString commandLine() const;
  QString desktopPath() const;
  QString desktopId() const;

  // static XdgDesktopApp fromAppId(QString appId);
  static XdgDesktopApp *fromDesktopFile(QString desktopFilePath,
                                        bool probeExecutable = true);
  virtual QJsonObject toJson() const override;
  static XdgDesktopApp *fromJson(QJsonObject obj);

private:
  XdgDesktopApp() {}
  XdgDesktopApp(QString p_id, QString p_desktopFilePath, QString p_executable,
                QString p_name, QString p_commandLine, QString p_description,
                QString p_iconName);

private:
  QString p_id;
  QString p_desktopFilePath;
  // base name of program, e.g. "anki"
  QString p_executable;
  QString p_name;
  QString p_commandLine;
  QString p_description;
  QString p_iconName;
  QIcon p_icon;
};

inline QString XdgDesktopApp::name() const { return p_name; }
inline QString XdgDesktopApp::commandLine() const { return p_commandLine; }
inline QString XdgDesktopApp::desktopPath() const { return p_desktopFilePath; }
inline QString XdgDesktopApp::desktopId() const { return p_id; }

} // namespace Tetradactyl
