#ifndef QWORKERMODULE_H
#define QWORKERMODULE_H

#include <QObject>
#include <QJSEngine>

namespace v8 {
    class Locker;
}

class QWorkerModule : public QObject
{
    Q_OBJECT
public:
    explicit QWorkerModule(QJSEngine *engine);
    ~QWorkerModule();
    
signals:
    
public slots:

    void start(const QJSValue &function);

    void yield();

private:
    QJSEngine *mEngine;
    v8::Locker *mLocker;
};

#endif // QWORKERMODULE_H
