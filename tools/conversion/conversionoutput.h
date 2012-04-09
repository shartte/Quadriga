#ifndef CONVERSIONRESULT_H
#define CONVERSIONRESULT_H

#include <QObject>
#include <QJSValue>
#include <QString>

class QCommonJSModule;

class ConversionOutput: public QObject
{
    Q_OBJECT
public:
    explicit ConversionOutput(QCommonJSModule *commonJsModule);
    
public slots:

    QJSValue addBuffer(const QString &category, const QString &filename, const QJSValue &buffer);

    QJSValue addString(const QString &category, const QString &filename, const QString &content);

private:
    QCommonJSModule *mCommonJsModule;
    
};

#endif // CONVERSIONRESULT_H
