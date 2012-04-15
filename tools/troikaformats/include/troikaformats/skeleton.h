#ifndef SKELETON_H
#define SKELETON_H

#include <QObject>
#include <QMatrix4x4>

namespace Troika
{

    class Skeleton;
    class SkeletonData;
    class MeshModel;
    struct Vertex;

    /**
      Models a single bone in a skeleton.
      */
    struct Bone
    {
        qint16 id;
        QByteArray name;
        qint16 parentId; // bone id of parent or -1
        QQuaternion rotation;
        QVector3D scale;
        QVector3D translation;
    };

    struct AnimationEvent
    {
        quint16 frameId; // When does the event happen
        QByteArray type; // Event type
        QByteArray action; // Event action
    };

    /**
      Describes the state of a bone while an animation is played.
      */
    struct AnimationBoneState
    {
        qint16 boneId;

        QQuaternion rotation;
        int rotationFrame;
        QVector3D translation;
        int translationFrame;
        QVector3D scale;
        int scaleFrame;

        QQuaternion nextRotation;
        int nextRotationFrame;
        QVector3D nextTranslation;
        int nextTranslationFrame;
        QVector3D nextScale;
        int nextScaleFrame;
    };

    /**
      An animation stream encapsulates the key frame stream of an animation.

      It allows forward navigation through the key frames.
      */
    class AnimationStream
    {
    public:
        AnimationStream(const QString &filename, const QByteArray &data, int dataStart, int boneCount, int frameCount);

        const AnimationBoneState *getBoneState(quint16 boneId) const;

        const QVector<AnimationBoneState> &boneStates() const;

        QByteArray calculateHash();

        bool atEnd() const;
        bool readNextFrame();
        void seek(int frame);
        void rewind();

        int getNextFrameId() const {
            return nextFrameId;
        }

    private:
        QString mFilename;
        int _dataStart; // Offset into the stream where the first key frame starts
        int _boneCount; // The total number of bones in the skeleton (highest bone id + 1)

        // Maps directly from bone-id to a pointer to the bone state for faster access.
        QScopedArrayPointer<AnimationBoneState*> _boneMap;

        // This array is initialized upon reading the first key-frame since the number
        // of bones affected by a key-frame stream is constant (and determined by the first
        // key frame)
        QVector<AnimationBoneState> _boneStates;

        QDataStream stream;

        float scaleFactor; // ToEE stores scale components as shorts and multiplies with this factor
        float translationFactor; // ToEE stores translation components as shorts and multiplies with this factor
        static const float rotationFactor; // ToEE uses a static factor for rotations
        qint16 nextFrameId; // Id of next key frame
                int frameCount;

        int countBones();
    };

    inline const QVector<AnimationBoneState> &AnimationStream::boneStates() const
    {
        return _boneStates;
    }

    inline bool AnimationStream::atEnd() const
    {
        return nextFrameId == -1 || stream.atEnd();
    }

    inline const AnimationBoneState *AnimationStream::getBoneState(quint16 boneId) const
    {
        Q_ASSERT(boneId < _boneCount);
        return _boneMap[boneId];
    }

    class Animation
    {
    public:
        /**
          How the animation progresses from frame to frame.
          */
        enum DriveType
        {
            Time = 0,
            Distance,
            Rotation
        };

        const QByteArray &name() const
        {
            return _name;
        }

        void setName(const QByteArray &name)
        {
            _name = name;
        }

        DriveType driveType() const
        {
            return _driveType;
        }

        void setDriveType(DriveType type)
        {
            _driveType = type;
        }

        bool loopable() const
        {
            return _loopable;
        }

        void setLoopable(bool loopable)
        {
            _loopable = loopable;
        }

        quint16 frames() const
        {
            return _frames;
        }

        void setFrames(quint16 frames)
        {
            _frames = frames;
        }

        float frameRate() const
        {
            return _frameRate;
        }

        void setFrameRate(float framesPerSecond)
        {
            _frameRate = framesPerSecond;
        }

        float dps() const
        {
            return _dps;
        }

        void setDps(float dps)
        {
            _dps = dps;
        }

        const QVector<AnimationEvent> &events() const
        {
            return _events;
        }

        void setEvents(const QVector<AnimationEvent> &events)
        {
            _events = events;
        }

        void setKeyFramesData(const QByteArray &data, quint32 dataStart)
        {
            _keyFramesData = data;
            _keyFramesDataStart = dataStart;
        }

                uint keyFramesDataStart() const {
                        return _keyFramesDataStart;
                }

        /**
          Opens a stream for the key frames of this animation.
          The caller takes ownership of the returned object.
          */
        AnimationStream *openStream(const Skeleton *skeleton) const;

        QDataStream *openRawStream() const;

        void freeRawStream(QDataStream *stream) const;

        void freeStream(AnimationStream *stream) const;

    private:
        QByteArray _name;
        DriveType _driveType;
        bool _loopable;
        quint16 _frames; // Number of frames
        float _frameRate; // FPS or total distance for movement
        float _dps; // Distance per second (both rotation & distance)
        quint32 _keyFramesDataStart; // The offset in the stream where the key frame data starts
        QByteArray _keyFramesData; // Animation data. Only used for reading key frames @ _keyFramesDataStart
        QVector<AnimationEvent> _events; // Events that are triggered by reaching a certain frame
    };

    class Skeleton
    {
    public:
        /**
          Constructs a skeleton from the associated SKA files data.

          @param bones The bones from the SKM model file.
          @param data The data of the SKA animation/skeleton file.
          */
        explicit Skeleton(const QByteArray &data, const QString &filename);
        ~Skeleton();

        const QVector<Bone> &bones() const;
        const QVector<Animation> &animations() const;

        const QString &filename() const;

        /**
          Finds the animation with the given name.
          @param name The name of the animation. Comparison is performed case insensitive.
          @returns Null if no animation with the given name could be found.
          */
        const Animation *findAnimation(const QByteArray &name) const;

    private:
        QScopedPointer<SkeletonData> d_ptr;
        Q_DISABLE_COPY(Skeleton);
    };

    /**
      This class encapsulates the state of a skeleton at a poin in time.
      Most importantly, it contains a complete transformation matrix for
      every bone in the skeleton that can be used directly to transform
      a vertex.
      */
    class SkeletonState
    {
    public:
        explicit SkeletonState(const Skeleton *skeleton);
        ~SkeletonState();

        QMatrix4x4 &getBoneMatrix(int boneId);
        const QMatrix4x4 &getBoneMatrix(int boneId) const;

    private:
        /**
          The matrices used to transform the bone.
          */
        const Skeleton *_skeleton;
        QScopedArrayPointer<QMatrix4x4> boneMatrices;

        Q_DISABLE_COPY(SkeletonState);
    };

    inline QMatrix4x4 &SkeletonState::getBoneMatrix(int boneId)
    {
        Q_ASSERT(boneId >= 0 && boneId < _skeleton->bones().size());
        return boneMatrices[boneId];
    }

    inline const QMatrix4x4 &SkeletonState::getBoneMatrix(int boneId) const
    {
        Q_ASSERT(boneId >= 0 && boneId < _skeleton->bones().size());
        return boneMatrices[boneId];
    }

}

#endif // SKELETON_H
