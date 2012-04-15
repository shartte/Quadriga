#include <QSharedDataPointer>
#include <QTime>

#include "troikaformats/skmreader.h"
#include "troikaformats/model.h"
#include "troikaformats/skeleton.h"

namespace Troika
{
    struct SkmHeader {
        quint32 boneCount;
        quint32 boneDataOffset;
        quint32 materialCount;
        quint32 materialDataOffset;
        quint32 vertexCount;
        quint32 vertexDataOffset;
        quint32 faceCount;
        quint32 faceDataOffset;
    };

    class SkmReaderData
    {
    public:
        QByteArray data;
        QString filename;

        SkmHeader header;

        // Data pointers that will be put into the resulting model
        QVector<BindingPoseBone> bones;
        QVector<Vertex> vertices;
        QList<QString> modelMaterials;
        QList< QList<Face> > faceGroupLists;
        QScopedPointer<Skeleton> skeleton;

        MeshModel *read() {
            QDataStream stream(data);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

            readHeader(stream);
            readBones(stream);
            readVertices(stream);
            readMaterials(stream);
            readFaces(stream);

            QList< QSharedPointer<FaceGroup> > faceGroups;
            for (quint32 i = 0; i < header.materialCount; ++i) {
                QVector<Face> faces = QVector<Face>::fromList(faceGroupLists[i]);

                QSharedPointer<FaceGroup> faceGroup(new FaceGroup(faces, modelMaterials[i]));
                faceGroups.append(faceGroup);
            }

            // qDebug("Loaded %s in %d ms.", qPrintable(filename), timer.elapsed());

            return new MeshModel(faceGroups, vertices, &bones, skeleton.take());
        }

        void readHeader(QDataStream &stream) {
            stream >> header.boneCount >> header.boneDataOffset
                    >> header.materialCount >> header.materialDataOffset
                    >> header.vertexCount >> header.vertexDataOffset
                    >> header.faceCount >> header.faceDataOffset;
        }

        void readBones(QDataStream &stream) {
            char rawName[49];
            rawName[48] = 0; // Force null termination

            stream.device()->seek(header.boneDataOffset);
            bones.resize(header.boneCount);

            for (quint32 i = 0; i < header.boneCount; ++i) {
                BindingPoseBone &bone = bones[i];
                bone.id = i;

                ushort flags; // Ignored, always 0
				short parentId;
                stream >> flags >> parentId;
				bone.parentId = parentId;
                stream.readRawData(rawName, 48);

                QMatrix4x4 &fullWorldInverse = bone.fullWorldInverse;

                // ToEE stores it matrix col-major, but with only 3 rows (last row is always 0001)
                for (int row = 0; row < 3; ++row) {
                    for (int col = 0; col < 4; ++col) {
                        stream >> fullWorldInverse(row, col);
                    }
                }

                bone.name = QByteArray(rawName);

                QMatrix4x4 fullWorld = fullWorldInverse.inverted();

                /* calculate the relative world matrix for this binding position bone */
                QMatrix4x4 relativeWorld;
                if (bone.parentId != -1)
                    relativeWorld = bones[bone.parentId].fullWorldInverse * fullWorld;
                else
                    relativeWorld = fullWorld;

                /* Extract translation, rotation and scale from the matrix */
                /* If non-uniform scale was used this completely breaks the calculation */
                bone.scale.setX(relativeWorld.column(0).length());
                bone.scale.setY(relativeWorld.column(1).length());
                bone.scale.setZ(relativeWorld.column(2).length());

                bone.position = relativeWorld.column(3).toVector3D();

                /* Un-scale the orthogonal 3x3 matrix */
                QVector4D rotCol1 = relativeWorld.column(0) / bone.scale.x();
                QVector4D rotCol2 = relativeWorld.column(1) / bone.scale.y();
                QVector4D rotCol3 = relativeWorld.column(2) / bone.scale.z();

                /* Re-build a rotation matrix out of it */
                QMatrix4x4 rotm;
                rotm.setColumn(0, rotCol1);
                rotm.setColumn(1, rotCol2);
                rotm.setColumn(2, rotCol3);

                float T = 1 + rotm(0,0) + rotm(1,1) + rotm(2,2); // Trace
                if (T > 0.00000001) {
                    float s = 0.5f/qSqrt(T);
                    bone.rotation.setScalar(0.25f/s);
                    bone.rotation.setX((rotm(2,1) - rotm(1,2))*s);
                    bone.rotation.setY((rotm(0,2) - rotm(2,0))*s);
                    bone.rotation.setZ((rotm(1,0) - rotm(0,1))*s);
                }
            }
        }

        void readVertices(QDataStream &stream) {
            stream.device()->seek(header.vertexDataOffset);

            vertices.resize(header.vertexCount);

            for (quint32 i = 0; i < header.vertexCount; ++i) {
                Vertex &vertex = vertices[i];

                stream  >> vertex.positionX >> vertex.positionY >> vertex.positionZ >> vertex.positionW
                        >> vertex.normalX >> vertex.normalY >> vertex.normalZ >> vertex.normalW
                        >> vertex.texCoordX >> vertex.texCoordY
                        >> vertex.padding
                        >> vertex.attachmentCount;
                for (int i = 0; i < 6; ++i) {
                    stream >> vertex.attachmentBone[i];
                }
                for (int i = 0; i < 6; ++i) {
                    stream >> vertex.attachmentWeight[i];
                }
            }
        }

        void readMaterials(QDataStream &stream)
        {
            char filename[129];
            filename[128] = 0; // Ensure that the string is null-terminated at all times

            stream.device()->seek(header.materialDataOffset);

            // Read the material filenames
            for (quint32 i = 0; i < header.materialCount; ++i) {
                stream.readRawData(filename, 128);

                modelMaterials.append(QString::fromLatin1(filename));
            }
        }

        void readFaces(QDataStream &stream)
        {
            struct FaceWithMaterial {
                quint16 materialId;
                Face face;
            } faceWithMaterial;
            Face &face = faceWithMaterial.face;
            quint16 &materialId = faceWithMaterial.materialId;

            // One face group per material
            for (quint32 i = 0; i < header.materialCount; ++i)
                faceGroupLists.append(QList<Face>());

            stream.device()->seek(header.faceDataOffset);

            for (quint32 i = 0; i < header.faceCount; ++i) {

                // The vertices are saved in the wrong order (since they're meant for DirectX)
                stream >> materialId >> face.vertices[0] >> face.vertices[1] >> face.vertices[2];

                faceGroupLists[materialId].append(face);
            }

        }

    };

    SkmReader::SkmReader(const QString &filename, const QByteArray &data) :
            d_ptr(new SkmReaderData)
    {
        d_ptr->filename = filename;
        d_ptr->data = data;
    }

    SkmReader::~SkmReader()
    {
    }

    MeshModel *SkmReader::get()
    {
        return d_ptr->read();
    }

}
