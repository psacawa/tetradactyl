#pragma once

#include <QLabel>

#include <string>

using std::string;

namespace Tetradactyl {
class HintLabel : public QLabel {
  Q_OBJECT
public:
  Q_PROPERTY(bool selected READ getSelected WRITE setSelected);

  // HintLabel(QWidget *_target, const char *label);
  HintLabel(string label, QWidget *_target);

  inline bool getSelected();
  inline void setSelected(bool);

private:
  bool selected;
};

inline bool HintLabel::getSelected() { return selected; }
inline void HintLabel::setSelected(bool _selected) { selected = _selected; }

} // namespace Tetradactyl
