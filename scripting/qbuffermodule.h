#ifndef QBUFFERMODULE_H
#define QBUFFERMODULE_H

#include "qcommonjsmodule.h"

class QJSEngine;

class QBufferModule
{
public:
    static void install(QCommonJSModule *commonJsModule);

    static QJSValue newBuffer(QCommonJSModule *commonJsModule, const QByteArray &data);

    static QByteArray getData(const QJSValue &buffer);

private:
    QBufferModule();
};

#endif // QBUFFERMODULE_H
