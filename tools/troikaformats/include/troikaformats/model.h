#ifndef MODEL_H
#define MODEL_H

#include <QtGlobal>
#include <QVector2D>
#include <QVector3D>
#include <QImage>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QtOpenGL/QtOpenGL>

#include "troikaformats/skeleton.h"
#include "util.h"

namespace Troika {

    class Material;

    const int MaxBoneAttachments = 6; // Maximum number of bones a vertex can be attached to
    const int VertexSize = 44 + MaxBoneAttachments * 6;

    /*
      Describes a single vertex including it's texture coordinate, normal and associated bone
      attachments for skeletal animation.
     */
    struct Vertex {
        float positionX;
        float positionY;
        float positionZ;
        float positionW; // Ignored
        float normalX;
        float normalY;
        float normalZ;
        float normalW; // Ignored
        float texCoordX;
        float texCoordY;
        quint16 padding;
        quint16 attachmentCount;
        quint16 attachmentBone[MaxBoneAttachments];
        float attachmentWeight[MaxBoneAttachments];

        QVector3D position() const
        {
            return QVector3D(positionX, positionY, positionZ);
        }

        QVector3D normal() const
        {
            return QVector3D(normalX, normalY, normalZ);
        }
    };

    /*
      Describes a single face, which in turn is defined by it's three vertices.
     */
    struct Face {
        /* Offsets of the associated vertices. This is organized in a fashion that should
                                        be vertex buffer object compatible. */
        quint16 vertices[3];
    };

    /*
      Models a face group.
     */
    class FaceGroup {
    public:
        FaceGroup(const QVector<Face> &faces, const QString &material);
        ~FaceGroup();

        /**
          Gets the pointer to the first face in this face group.
          */
        const QVector<Face> &faces() const {
            return _faces;
        }

        const QString &material() const {
            return _material;
        }

    protected:
        QVector<Face> _faces;
        int _count;
        QString _material;

        Q_DISABLE_COPY(FaceGroup)
    };

    struct BindingPoseBone {
        int id;
        int parentId;
        QByteArray name;
        QMatrix4x4 fullWorldInverse;
        QMatrix4x4 relativeWorld;
        QVector3D position; /* Relative to parent */
        QVector3D scale; /* Relative to parent */
        QQuaternion rotation; /* Relative to parent */
    };

    class MeshModel
    {
    public:
        /*
          Constructs a new mesh model from face groups, vertices and bones.
          The mesh model takes ownership of the given pointers.
         */
        MeshModel(QList< QSharedPointer<FaceGroup> > faceGroups,
                  const QVector<Vertex> &vertices,
                  const QVector<BindingPoseBone> *bindingPose = NULL,
                  Skeleton *skeleton = NULL);
        ~MeshModel();

        const QList< QSharedPointer<FaceGroup> > &faceGroups() const {
            return _faceGroups;
        }

        const QVector<Vertex> &vertices() const {
            return _vertices;
        }

        /**
          Gets a pointer to the skeleton associated with this object.
          The pointer will be valid as long as this object is valid.
          NULL will be returned, if there is no skeleton associated
          with this mesh.
          TODO: Check if it would be better to return an "empty" singleton skeleton to
              make life easier.
          */
        const Skeleton *skeleton() const
        {
            return _skeleton.data();
        }

        void setSkeleton(Skeleton *skeleton)
        {
            _skeleton.reset(skeleton);
        }

        /**
          Gets the axis aligned bounding box of this object.
          Please note that this bounding box is not transformed by any animations and only includes
          the initial pose of the object. It *would* be better to have a bounding box that
          includes the maximum dimension of the object at any time during any animation, but sadly
          this information is not available.
          */
        const Box3D &boundingBox() const {
            return _boundingBox;
        }

        GLuint createVBO() const;

        void draw(const SkeletonState *skeletonState) const;
        void draw() const;
        void saveAsText(const QString &filename);

        const QVector<BindingPoseBone> &bindingPoseBones() const
        {
            return mBones;
        }

    private:
        Box3D _boundingBox;
        void createBoundingBox();
        void createBuffers();

        QVector<BindingPoseBone> mBones;

        QList< QSharedPointer<FaceGroup> > _faceGroups;
        QVector<Vertex> _vertices;
        QScopedPointer<Skeleton> _skeleton;
        GLuint vertexBuffer;
        GLuint indexBuffer;

        Q_DISABLE_COPY(MeshModel)
    };

}

#endif // MODEL_H
