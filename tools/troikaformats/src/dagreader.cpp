
#include "troikaformats/dagreader.h"
#include "troikaformats/model.h"

#include <QDataStream>

namespace Troika
{

    MeshModel *DagReader::read(const QByteArray &data)
    {
        if (data.isNull())
            return nullptr;

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        /*
         ToEE uses a bounding sphere for depth art geometry.

         float - X of the bounding sphere center offset
         float - Y of the bounding sphere center offset
         float - Z of the bounding sphere center offset
         float - Radius of the bounding sphere
         */
        stream.skipRawData(4 * sizeof(float));

        qint32 objectCount, dataStart;
        stream >> objectCount >> dataStart; // The format allows for more than one object.
        Q_ASSERT(objectCount == 1); // But we can only allow one object per file
        // TODO: Can we assert that dataStart == stream.device()->pos() ?

        quint32 vertexCount; // Number of vertices
        quint32 faceCount; // Number of triangles
        quint32 vertexDataStart; // Offset in the file where the vertex data starts
        quint32 faceDataStart; // Offset in the file where the face data starts

        stream >> vertexCount >> faceCount >> vertexDataStart >> faceDataStart;

        // Read the vertices
        stream.device()->seek(vertexDataStart);

        QVector<Vertex> vertices(vertexCount);

        for (quint32 i = 0; i < vertexCount; ++i) {
            Vertex &vertex = vertices[i];
            stream >> vertex.positionX >> vertex.positionZ >> vertex.positionY;
            vertex.positionX = - vertex.positionX;
            vertex.positionZ = - vertex.positionZ;
            vertex.attachmentCount = 0;
        }

        // Read the faces
        stream.device()->seek(faceDataStart);

        QVector<Face> faces(faceCount);

        for (quint32 i = 0; i < faceCount; ++i) {
            Face &face = faces[i];
            stream >> face.vertices[0] >> face.vertices[1] >> face.vertices[2];
        }

        // Create a single virtual face group
        QSharedPointer<FaceGroup> faceGroup( new FaceGroup(faces, "DEPTH_ART" ) );

        QList< QSharedPointer<FaceGroup> > faceGroups;
        faceGroups.append(faceGroup);

        return new MeshModel(faceGroups, vertices);
    }

}
