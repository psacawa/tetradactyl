// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <QMutex>
#include <QtGlobal>

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
using TetraMutexLocker = QMutexLocker<QMutex>;
#else
using TetraMutexLocker = QMutexLocker;
#endif
