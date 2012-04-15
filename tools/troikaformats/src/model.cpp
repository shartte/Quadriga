
#include "troikaformats/model.h"

namespace Troika {

    MeshModel::MeshModel(QList< QSharedPointer<FaceGroup> > faceGroups,
                         const QVector<Vertex> &vertices,
                         const QVector<BindingPoseBone> *bindingPose,
                         Skeleton *skeleton) :
    _faceGroups(faceGroups),
    _vertices(vertices),
    _skeleton(skeleton)
    {
        if (bindingPose) {
            mBones = *bindingPose;
        }

        createBoundingBox();
    }

    MeshModel::~MeshModel()
    {
    }

    void MeshModel::createBoundingBox()
    {
        if (!_vertices.isEmpty())
        {
            float minX, minY, minZ;
            float maxX, maxY, maxZ;

            const Vertex &firstVertex = _vertices[0];
            minX = maxX = firstVertex.positionX;
            minY = maxY = firstVertex.positionY;
            minZ = maxZ = firstVertex.positionZ;

            for (int i = 1; i < _vertices.count(); ++i)
            {
                const Vertex &vertex = _vertices[i];

                minX = qMin(minX, vertex.positionX);
                maxX = qMax(maxX, vertex.positionX);

                minY = qMin(minY, vertex.positionY);
                maxY = qMax(maxY, vertex.positionY);

                minZ = qMin(minZ, vertex.positionZ);
                maxZ = qMax(maxZ, vertex.positionZ);
            }

            _boundingBox.setExtents(QVector3D(minX, minY, minZ), QVector3D(maxX, maxY, maxZ));
        }
    }

    FaceGroup::FaceGroup(const QVector<Face> &faces, const QString &material) :
            _faces(faces), _material(material) {
    }

    FaceGroup::~FaceGroup() {
    }

}
