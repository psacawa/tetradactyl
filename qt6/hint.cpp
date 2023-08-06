#include <QFile>
#include <QIODevice>
#include <QLabel>
#include <QString>
#include <QStyle>

#include <string>

#include "hint.h"

using std::string;

HintLabel::HintLabel(string label, QWidget *_target)
    : QLabel(label.c_str(), _target) {}

#include "moc_hint.cpp"
