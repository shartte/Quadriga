#ifndef OGREMODELCONVERTER_H
#define OGREMODELCONVERTER_H

#include <QObject>
#include <QJSValue>

class QCommonJSModule;

class OgreModelConverter : public QObject
{
    Q_OBJECT
public:
    explicit OgreModelConverter(QCommonJSModule *commonJs);
    ~OgreModelConverter();

public slots:
    QJSValue convertModel(QJSValue modelData, QJSValue skeletonData);
    
private:
    QCommonJSModule *mCommonJs;

};

#endif // OGREMODELCONVERTER_H
