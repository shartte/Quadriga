
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

bool QBufferModule::isBuffer(const QJSValue &buffer)
{
    v8::Handle<v8::Value> bufferObject = *QJSValuePrivate::get(buffer);
    return Buffer::HasInstance(bufferObject);
}

QJSValue QBufferModule::newBuffer(QCommonJSModule *commonJsModule, const QByteArray &data)
{
    auto bufferModule = commonJsModule->require("buffer");
    auto bufferConstructor = bufferModule.property("Buffer");

    QV8Engine *v8Engine = QV8Engine::get(commonJsModule->engine());

    Buffer *buffer = Buffer::New(data.data(), data.size());
    QJSValue slowBuffer = v8Engine->scriptValueFromInternal(buffer->handle_);
    int len = Buffer::Length(buffer);

    return bufferConstructor.callAsConstructor(QJSValueList() << slowBuffer << len << QJSValue());
}

QByteArray QBufferModule::getData(const QJSValue &buffer)
{
    v8::Handle<v8::Object> bufferObject = *QJSValuePrivate::get(buffer);

    int len = Buffer::Length(bufferObject);
    char *data = Buffer::Data(bufferObject);

    return QByteArray::fromRawData(data, len);
}
