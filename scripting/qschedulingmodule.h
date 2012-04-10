#ifndef QSCHEDULINGMODULE_H
#define QSCHEDULINGMODULE_H

#include <QObject>
#include <QJSEngine>
#include <QJSValue>

class QSchedulingModule : public QObject
{
    Q_OBJECT
public:
    explicit QSchedulingModule(QJSEngine *engine);

private slots:
    void doCall(QJSValue callback);
    void doSequenceCall(QJSValue sequence, QJSValue lastResult, int index);

public slots:
    /**
     * @brief Defers a function call until the next event loop iteration runs.
     * @param value The function call to defer.
     */
    void defer(QJSValue callback);

private:
    QJSEngine *mEngine;
};

#endif // QSCHEDULINGMODULE_H
