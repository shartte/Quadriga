#include "ogremodelconverter.h"
#include "qcommonjsmodule.h"
#include "qbuffermodule.h"

#include <Ogre.h>

OgreModelConverter::OgreModelConverter(QCommonJSModule *commonJs) :
    QObject(commonJs), mCommonJs(commonJs)
{
}

OgreModelConverter::~OgreModelConverter()
{
}

QJSValue OgreModelConverter::convertModel(QJSValue modelDataJs, QJSValue skeletonDataJs)
{
    QByteArray modelData = QBufferModule::getData(modelDataJs);
    QByteArray skeletonData = QBufferModule::getData(skeletonDataJs);

    return QJSValue();

}
