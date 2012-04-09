
#include <QJSEngine>


#include <QtQml/private/qjsvalue_impl_p.h>
#include <QtQml/private/qjsvalue_p.h>
#include <QtQml/private/qv8engine_p.h>

#include "qbuffermodule.h"
#include "v8_buffer.h"

void QBufferModule::install(QCommonJSModule *commonJsModule)
{
    QJSEngine *engine = commonJsModule->engine();

    v8::Persistent<v8::Context> context = v8::Context::New();
    context->Enter();

    QJSValue module = engine->newObject();

    /* Use the underlying v8 object to initialize the Buffer module */
    v8::Handle<v8::Object> obj = *QJSValuePrivate::get(module);
    Buffer::Initialize(obj);

    commonJsModule->addModule("native/buffer", module);

    context->Exit();

}

QJSValue QBufferModule::newBuffer(QJSEngine *engine, const QByteArray &data)
{
    QV8Engine *v8Engine = QV8Engine::get(engine);

    Buffer *buffer = Buffer::New(data.data(), data.size());

    return v8Engine->scriptValueFromInternal(buffer->handle_);
}
