#ifndef CLIPPINGGEOMETRYCONVERTER_H
#define CLIPPINGGEOMETRYCONVERTER_H

#include <QByteArray>
#include <QJSValue>

class OgreSystems;
class IConversionOutput;
class ClippingConverterPrivate;
class QCommonJSModule;

class ClippingConverter : public QObject
{
Q_OBJECT
public:
    ClippingConverter(QCommonJSModule *commonJs, OgreSystems *ogreSystems);
    ~ClippingConverter();

public slots:
    QJSValue convert(const QJSValue &clippingData);

private:
    OgreSystems *mOgreSystems;
    QCommonJSModule *mCommonJs;
};

#endif // CLIPPINGGEOMETRYCONVERTER_H
