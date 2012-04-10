#include <QtDebug>

#include "qschedulingmodule.h"

QSchedulingModule::QSchedulingModule(QJSEngine *engine) :
    QObject(engine), mEngine(engine)
{
}

void QSchedulingModule::doCall(QJSValue callback)
{
    callback.call();

    if (mEngine->hasUncaughtException()) {
        qDebug() << "Deferred callback caused unhandeled exception:" << mEngine->uncaughtException().toString();
        mEngine->clearExceptions();
    }
}

void QSchedulingModule::doSequenceCall(QJSValue sequence, QJSValue lastResult, int index)
{

    QJSValue callback = sequence.property(index);

    if (callback.isUndefined()) {
        // Callback sequence ended
        return;
    }

    lastResult = callback.call(QJSValueList() << lastResult);

    if (mEngine->hasUncaughtException()) {
        qDebug() << "While processing callback sequence an error occured @"
                 << index << ":" << mEngine->uncaughtException().toString();
        return;
    }

    QMetaObject::invokeMethod(this, "doSequenceCall", Qt::QueuedConnection, Q_ARG(QJSValue, sequence), Q_ARG(QJSValue, lastResult), Q_ARG(int, index + 1));

}

void QSchedulingModule::defer(QJSValue callback)
{
    if (callback.isArray()) {
        QMetaObject::invokeMethod(this, "doSequenceCall", Qt::QueuedConnection, Q_ARG(QJSValue, callback), Q_ARG(QJSValue, QJSValue()), Q_ARG(int, 0));
    }

    if (!callback.isObject()) {
        qDebug() << "Trying to defer a function call with a callback that is not a function.";
        return; // TODO: Exception
    }

    QMetaObject::invokeMethod(this, "doCall", Qt::QueuedConnection, Q_ARG(QJSValue, callback));
}
