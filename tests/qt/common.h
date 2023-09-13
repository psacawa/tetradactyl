// Copyright 2023 Paweł Sacawa. All rights reserved. 

#pragma once
#include <QWidget>

void waitForWindowActiveOrFail(QWidget *);

// default to offscreen rendering
static void __attribute__((constructor)) setupDefaultQtPlatform() {
  if (qgetenv("QT_QPA_PLATFORM") == "")
    qputenv("QT_QPA_PLATFORM", "offscreen");
}
