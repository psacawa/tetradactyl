// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QLineEdit>

namespace Tetradactyl {

class Overlay;

class CommandLine : public QLineEdit {
  Q_OBJECT
public:
  Q_PROPERTY(bool opened READ isOpen WRITE setOpened)
  CommandLine(QWidget *parent = nullptr);
  virtual ~CommandLine() {}

  bool isOpen();
  void setOpened(bool);

  void focusOutEvent(QFocusEvent *) override;
  void focusInEvent(QFocusEvent *) override;
  void reportError();

  QSize minimumSizeHint() const override;

signals:
  // This doesn't guarantee that this was a valid command line
  void accepted(QString cmdline);
  void opened();
  void closed();

public slots:
  void open();

private:
  bool p_isOpen;
};

inline bool CommandLine::isOpen() { return p_isOpen; }

inline QSize CommandLine::minimumSizeHint() const {
  QSize ret = QLineEdit::minimumSizeHint();
  ret.setWidth(window()->width());
  return ret;
};

} // namespace Tetradactyl
