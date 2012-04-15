#ifndef CONVERTERS_MESHCONVERTER_H
#define CONVERTERS_MESHCONVERTER_H

#include <QJSValue>
#include <QByteArray>

#include "ogresystems.h"

namespace Troika {
    class MeshModel;
}

class QCommonJSModule;

class ModelConverter : public QObject
{
Q_OBJECT
public:
    ModelConverter(QCommonJSModule *commonJs, OgreSystems *ogreSystems);

public slots:
    QJSValue convert(QJSValue meshData, QJSValue skeletonData, QJSValue materialNameCallback, QJSValue skeletonNameCallback);

private:
    QCommonJSModule *mCommonJs;
    OgreSystems *mOgreSystems;

    QString getMaterialName(QJSValue materialNameCallback, const QString &filename);
    QString getSkeletonName(QJSValue skeletonNameCallback, const QString &filename);

    QByteArray convertMesh(const Troika::MeshModel &model, QJSValue materialNameCallback, QJSValue skeletonNameCallback);

    QByteArray convertSkeleton(const Troika::MeshModel &model);

    void postprocessMesh(Ogre::Mesh *mesh);

    /* Mesh imports */
    void importMesh(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel, QJSValue materialNameCallback, QJSValue skeletonNameCallback);
    void importGeometry(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel);
    void importSubmeshes(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel, QJSValue materialNameCallback);
    void importBoneAssignments(Ogre::Mesh *ogreMesh, const Troika::MeshModel &troikaModel);

    /* Skeleton imports */
    void importSkeleton(Ogre::Skeleton *ogreSkeleton, const Troika::MeshModel &troikaModel);
    void importBones(Ogre::Skeleton *ogreSkeleton, const Troika::MeshModel &troikaModel);
    void importAnimations(Ogre::Skeleton *ogreSkeleton, const Troika::MeshModel &troikaModel);
};

#endif // CONVERTERS_MESHCONVERTER_H
