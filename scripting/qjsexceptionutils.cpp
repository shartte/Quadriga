
#include <QtDebug>

#include "qjsexceptionutils.h"

#include <QtQml/private/qv8engine_p.h>
#include <QtQml/private/qv8engine_impl_p.h>
#include <QtQml/private/qjsvalue_p.h>
#include <QtV8/private/v8.h>

QJSValue QJSExceptionUtils::newError(QJSEngine *engine, const QString &message)
{
    QV8Engine *v8Engine = QV8Engine::get(engine);

    v8::Local<v8::Value> v8Message = v8Engine->makeJSValue(message);
    v8::Handle<v8::String> v8MessageString = v8::Handle<v8::String>::Cast(v8Message);

    return v8Engine->scriptValueFromInternal(v8::ThrowException(v8::Exception::Error(v8MessageString)));
}

void QJSExceptionUtils::handleUnhandledException(QJSEngine *engine)
{
    if (!engine->hasUncaughtException())
        return;

    QJSValue uncaughtException = engine->uncaughtException();

    qDebug() << "Uncaught Exception" << uncaughtException.toString() << uncaughtException.property("stack").toString();

    engine->clearExceptions();

}
