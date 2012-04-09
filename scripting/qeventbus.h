#ifndef QEVENTBUS_H
#define QEVENTBUS_H

#include <QList>
#include <QHash>
#include <QJSValue>
#include <QObject>

class QEventBus : public QObject
{
    Q_OBJECT
public:
    explicit QEventBus(QObject *parent = 0);

signals:
    
public slots:

    void publish(const QString &channel, const QJSValue &message);
    void subscribe(const QString &channel, const QJSValue &callback);

private:
    QHash<QString, QList<QJSValue>> mSubscriptions;
    
};

Q_DECLARE_METATYPE(QEventBus*)

#endif // QEVENTBUS_H
