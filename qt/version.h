// Copyright 2023 Paweł Sacawa. All rights reserved. 
#include <QtGlobal>

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
#define TETRA_MUTEX_LOCKER QMutexLocker<QMutex>
#else
#define TETRA_MUTEX_LOCKER QMutexLocker
#endif

