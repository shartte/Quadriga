
#include <QtV8/private/v8.h>
#include <QtV8/private/v8.h>

#include <QtQml/private/qjsvalue_p.h>
#include <QtQml/private/qjsvalue_impl_p.h>

#include <QThread>
#include <QDebug>
#include <QTimer>

#include "qworkermodule.h"

class QJSWorkerThread : public QThread {
public:
    QJSWorkerThread(QWorkerModule *module, QJSEngine *engine, const QJSValue &function)
        : QThread(module), mModule(module), mEngine(engine), mFunction(function), mIsolate(v8::Isolate::GetCurrent())
    {
    }

    void run();
private:
    v8::Isolate *mIsolate;
    QJSValue mFunction;
    QJSEngine *mEngine;
    QWorkerModule *mModule;
};

QWorkerModule::QWorkerModule(QJSEngine *engine) :
    QObject(engine), mEngine(engine), mLocker(new v8::Locker())
{
    qDebug() << "Locker is now" << v8::Locker::IsActive();
    v8::Locker::StartPreemption(10);
    qDebug() << "Started preemption";

    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), SLOT(yield()));
    timer->start();
}

QWorkerModule::~QWorkerModule()
{
    delete mLocker;
}

void QWorkerModule::start(const QJSValue &function)
{
    /* Run the function in a new thread */
    QJSWorkerThread *workerThread = new QJSWorkerThread(this, mEngine, function);
    workerThread->start();
}

void QWorkerModule::yield()
{
    v8::Unlocker unlocker;
    QThread::yieldCurrentThread();
}

void QJSWorkerThread::run() {

    qDebug() << "1 Locker is now" << v8::Locker::IsActive();
    v8::Locker locker(mIsolate);
    qDebug() << "2 Locker is now" << v8::Locker::IsActive();

    mIsolate->Enter();

    v8::Persistent<v8::Context> context = v8::Context::New();

    qDebug() << "Locker is:" << v8::Locker::IsActive();

    QJSValue workerModule = mEngine->newQObject(mModule);

    context->Enter();

    /* The handle scope's destructor must run before the isolation is exited. */
    {
        v8::HandleScope handleScope;

        v8::Handle<v8::Value> value = *QJSValuePrivate::get(mFunction);
        v8::Local<v8::Function> function(v8::Function::Cast(*value));

        v8::Handle<v8::Value> arg = *QJSValuePrivate::get(workerModule);

        v8::Handle<v8::Object> object = *QJSValuePrivate::get(mFunction);
        function->Call(object, 1, &arg);

        if (mEngine->hasUncaughtException()) {
            qDebug() << "Uncaught Exception for Worker: " << mEngine->uncaughtException().toString();
        }

        qDebug() << handleScope.NumberOfHandles();
    }

    context->Exit();

    mIsolate->Exit();

}
