
#include <QDebug>
#include <QThread>
#include <QMetaMethod>
#include "qeventbus.h"

QEventBus::QEventBus(QObject *parent) :
    QObject(parent)
{
}

void QEventBus::publish(const QString &channel, const QJSValue &message)
{
    if (thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, "publish", Qt::QueuedConnection, Q_ARG(QString, channel), Q_ARG(QJSValue, message));
    } else {
        QList<QJSValue> subscriptions = mSubscriptions[channel];
        foreach (QJSValue subscription, subscriptions) {
            subscription.call(QJSValueList() << channel << message);
        }
    }
}

void QEventBus::subscribe(const QString &channel, const QJSValue &callback)
{
    qDebug() << "Subscribe" << channel << "with callback" << callback.toString();
    mSubscriptions[channel].append(callback);
}
