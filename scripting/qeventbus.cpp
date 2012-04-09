
#include <QDebug>
#include "qeventbus.h"

QEventBus::QEventBus(QObject *parent) :
    QObject(parent)
{
}

void QEventBus::publish(const QString &channel, const QJSValue &message)
{
    qDebug() << "Publish to" << channel << "message" << message.toString();
    QList<QJSValue> subscriptions = mSubscriptions[channel];
    foreach (QJSValue subscription, subscriptions) {
        subscription.call(QJSValueList() << channel << message);
    }
}

void QEventBus::subscribe(const QString &channel, const QJSValue &callback)
{
    qDebug() << "Subscribe" << channel << "with callback" << callback.toString();
    mSubscriptions[channel].append(callback);
}
