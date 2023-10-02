// Copyright 2023 Paweł Sacawa. All rights reserved.
#pragma once
#include <QLineEdit>

namespace Tetradactyl {

class Overlay;

class CommandLine : public QLineEdit {
  Q_OBJECT
public:
  CommandLine(QWidget *parent = nullptr);
  virtual ~CommandLine() {}

  void focusOutEvent(QFocusEvent *) override;
  void focusInEvent(QFocusEvent *) override;
  void reportError();

  QSize minimumSizeHint() const override;

signals:
  void accepted(QString cmd);

public slots:
  void open();
};

inline QSize CommandLine::minimumSizeHint() const {
  QSize ret = QLineEdit::minimumSizeHint();
  ret.setWidth(window()->width());
  return ret;
};

} // namespace Tetradactyl
