#ifndef QBUFFERMODULE_H
#define QBUFFERMODULE_H

#include "qcommonjsmodule.h"

class QJSEngine;

class QBufferModule
{
public:
    static void install(QCommonJSModule *commonJsModule);

    static QJSValue newBuffer(QJSEngine *engine, const QByteArray &data);

private:
    QBufferModule();
};

#endif // QBUFFERMODULE_H
