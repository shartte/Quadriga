
#include "troikaformats/skeleton.h"
#include "troikaformats/model.h"

#include <QCryptographicHash>
#include <QDataStream>

namespace Troika
{

    static inline QByteArray normalizeBoneName(QByteArray boneName)
    {
        boneName = boneName.toLower();

        // Strip the _ground from some bone names, although this is most likely irrelevant anyway
        // since those skeletons are mostly empty, and in that case we really don't need bones.
        // Unless they're ref bones that is.
        if (boneName.startsWith("#cylinder")
            || boneName.startsWith("#sphere")) {
            int indexOfBracket = boneName.indexOf(']');
            return boneName.left(indexOfBracket + 2);
        }

        return boneName;
    }

    QDataStream *Animation::openRawStream() const
    {
        QDataStream *result = new QDataStream(_keyFramesData);
        result->device()->seek(_keyFramesDataStart);
        result->setByteOrder(QDataStream::LittleEndian);
        result->setFloatingPointPrecision(QDataStream::SinglePrecision);
        return result;
    }

    AnimationStream *Animation::openStream(const Skeleton *skeleton) const
    {
        int boneCount = skeleton->bones().size();
        if (_frames == 0)
            return 0;
        else
            return new AnimationStream(skeleton->filename(), _keyFramesData, _keyFramesDataStart, boneCount, _frames);
    }

    void Animation::freeRawStream(QDataStream *stream) const
    {
        delete stream;
    }

    void Animation::freeStream(AnimationStream *stream) const
    {
        delete stream;
    }

    class SkeletonData
    {
    public:
        QByteArray data; // SKA data
        QDataStream stream;
        QString filename;

        void loadBones(quint32 count, quint32 dataStart)
        {
            stream.device()->seek(dataStart);

            bones.resize(count);
            for (quint32 i = 0; i < count; ++i) {
                loadBone(i);
            }
        }

        void loadBone(quint32 boneId)
        {
            qint16 flags; // Bone flags (ignored)
            qint16 parentId; // Id of parent bone
            stream >> flags >> parentId;

            char rawBoneName[49]; // Name of bone + null termination
            rawBoneName[48] = 0;
            stream.readRawData(rawBoneName, 48);

            if (flags != 0) {
                qDebug("Bone flags in SKA %s for bone %s are: %x", qPrintable(filename), rawBoneName, flags);
            }

            // Bones in the skeleton file are associated with bones from the original model
            // It's possible that the skeleton defines additional bones.
            Bone &bone = bones[boneId];
            bone.id = boneId;
            bone.name = QByteArray(rawBoneName);
            bone.parentId = parentId;

            QVector4D scale; // The fourth component is discarded
            QVector4D rotation; // Order is: X,Y,Z,Scalar
            QVector4D translation; // The fourth component is discarded

            stream >> scale >> rotation >> translation;

            // Build the relative world matrix for the bone
            bone.translation = translation.toVector3D();
            bone.rotation = QQuaternion(rotation.w(), rotation.x(), rotation.y(), rotation.z());
            bone.scale = scale.toVector3D();
        }

        void loadAnimations(quint32 count, quint32 dataStart)
        {
            animations.resize(count);

            for (quint32 i = 0; i < count; ++i)
            {
                loadAnimation(i, dataStart);
            }

        }

        void loadAnimation(quint32 animationId, quint32 dataStart)
        {
            // 0xEC is the size of an entire animation header with all streams (of which only one is used).
            quint64 animDataStart = dataStart + animationId * 0xEC;

            stream.device()->seek(animDataStart);

            Animation &animation = animations[animationId];

            char rawName[65];
            rawName[64] = 0;
            stream.readRawData(rawName, 64);

            animation.setName(QByteArray(rawName));

            quint8 driveType;
            quint8 loopable;
            quint16 eventCount;
            quint32 eventOffset;
            quint32 streamCount;
            qint16 variationId;
            quint32 dataOffset; // Offset to the start of the key-frame stream in relation to animationStart
            quint16 frames;
            float frameRate;
            float dps;

            stream  >> driveType >> loopable >> eventCount >> eventOffset >> streamCount
                    >> frames >> variationId >> frameRate >> dps >> dataOffset;

            Q_ASSERT(streamCount == 0 || streamCount == 1); // Only one stream is supported

            // TODO: Special handling for "empty" animations
            if (streamCount == 0) {
                frames = 0;
                variationId = -1;
            }

            Q_ASSERT(variationId == -1); // Variations are unsupported

            animation.setDriveType( static_cast<Animation::DriveType>(driveType) );
            animation.setLoopable(loopable != 0);
            animation.setKeyFramesData(data, animDataStart + dataOffset);
            animation.setFrames(frames);
            animation.setFrameRate(frameRate);
            animation.setDps(dps);

            animationMap.insert(animation.name().toLower(), &animation);

            // Read Events
            loadEvents(animation, eventCount, animDataStart + eventOffset);
        }

        void loadEvents(Animation &animation, quint32 count, quint32 dataStart)
        {
            stream.device()->seek(dataStart);

            QVector<AnimationEvent> events(count);

            char type[49];
            type[48] = 0;
            char action[129];
            action[128] = 0;

            for (quint32 i = 0; i < count; ++i) {
                AnimationEvent &event = events[i];

                stream >> event.frameId;
                stream.readRawData(type, 48);
                stream.readRawData(action, 128);

                event.type = QByteArray(type);
                event.action = QByteArray(action);
            }

            animation.setEvents(events);
        }

    public:
        QVector<Bone> bones;
        QVector<Animation> animations;
        QHash<QByteArray, Animation*> animationMap;

        SkeletonData(const QByteArray &_data, const QString &_filename)
            : data(_data), stream(data), filename(_filename)
        {
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        }

        /**
          Load the initial data from the stream.
          */
        void load()
        {
            if (data.isNull()) {
                return; // No animation data present.
            }

            quint32 boneCount; // Number of bones in skeleton
            quint32 bonesDataStart; // Start of bones data in bytes
            quint32 variationCount; // Unknown + Unused
            quint32 variationsDataStart; // Unknown + Unused
            quint32 animationCount; // Number of animations in this skeletons
            quint32 animationsDataStart; // Offset to first animation header

            stream  >> boneCount >> bonesDataStart
                    >> variationCount >> variationsDataStart
                    >> animationCount >> animationsDataStart;

            // Assert that this file doesn't contain any variations,
            // since their format is unknown.
            Q_ASSERT(variationCount == 0);

            loadBones(boneCount, bonesDataStart);
            loadAnimations(animationCount, animationsDataStart);
        }
    };

    Skeleton::Skeleton(const QByteArray &data, const QString &filename)
                           : d_ptr(new SkeletonData(data, filename))
    {
        d_ptr->load();
    }

    Skeleton::~Skeleton()
    {
    }

    const QString &Skeleton::filename() const
    {
        return d_ptr->filename;
    }

    const QVector<Bone> &Skeleton::bones() const
    {
        return d_ptr->bones;
    }

    const QVector<Animation> &Skeleton::animations() const
    {
        return d_ptr->animations;
    }

    const Animation *Skeleton::findAnimation(const QByteArray &name) const
    {
        QHash<QByteArray,Animation*>::const_iterator it = d_ptr->animationMap.find(name.toLower());
        if (it == d_ptr->animationMap.end()) {
            return 0;
        } else {
            return it.value();
        }
    }

    SkeletonState::SkeletonState(const Skeleton *skeleton) : _skeleton(skeleton), boneMatrices((QMatrix4x4*)0) {
        boneMatrices.reset(new QMatrix4x4[skeleton->bones().size()]);
    }

    SkeletonState::~SkeletonState() {
    }

    const float AnimationStream::rotationFactor = 1 / 32766.0f;

    AnimationStream::AnimationStream(const QString &filename,
                                     const QByteArray &data,
                                     int dataStart,
                                     int boneCount,
                                     int _frameCount)
        : _dataStart(dataStart), _boneCount(boneCount),
        _boneMap(new AnimationBoneState*[boneCount]), stream(data), mFilename(filename), frameCount(_frameCount)
    {
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        stream.device()->seek(dataStart);

        // Read float scaling factors once
        stream >> scaleFactor >> translationFactor;

        _boneStates.resize(countBones()); // Avoid memory reallocation
        memset(_boneMap.data(), 0, sizeof(AnimationBoneState*) * boneCount);

        rewind();
    }

    int AnimationStream::countBones()
    {
        qint16 boneId;
        int bones = 0;

        stream >> boneId;

        while (boneId != -2) {
            ++bones;

            stream.skipRawData((3 + 3 + 4) * 2); // Skip the bone data

            stream >> boneId;
        }

        return bones;
    }

    QByteArray AnimationStream::calculateHash()
    {
        QCryptographicHash hash(QCryptographicHash::Md5);

        qint64 start = _dataStart + 2 * sizeof(float);

        rewind();

        while (nextFrameId != -1)
            readNextFrame();

        qint64 end = stream.device()->pos();

        rewind();

        QBuffer *buffer = qobject_cast<QBuffer*>(stream.device());

        Q_ASSERT(buffer);

        const char *hashStart = buffer->data().constData() + start;

        hash.addData(hashStart, end - start);

        return hash.result();
    }

    bool AnimationStream::readNextFrame()
    {
        if (nextFrameId == -1) {
            // Make all "next states" current and keep them forever
            for (int i = 0; i < _boneStates.size(); ++i) {
                AnimationBoneState &state = _boneStates[i];
                state.scale = state.nextScale;
                state.scaleFrame = state.nextScaleFrame;
                state.rotation = state.nextRotation;
                state.rotationFrame = state.nextRotationFrame;
                state.translation = state.nextTranslation;
                state.translationFrame = state.nextTranslationFrame;
            }

            return true;
        }

        qint16 boneHeader;

        // Read Bone Chunks of Key Frame
        stream >> boneHeader;

        // The LSB is the flag whether this is a bone-chunk or the id of the next frame
        while ((boneHeader & 1) == 1) {
            int boneId = boneHeader >> 4;

            if (boneId < 0 || boneId >= _boneCount) {
                qWarning("Animation stream in %s tries to reference an invalid bone.", qPrintable(mFilename));
                return false;
            }

            // Get a pointer to the bone state affected by this key frame chunk
            AnimationBoneState *state = _boneMap[boneId];

            // Would've been too nice Q_ASSERT(state);

            if (!state) {
                qWarning("Found key-frame data in frame %d for bone %d of file %s, which isn't part of "
                         "the initial key frame.", nextFrameId, boneId, qPrintable(mFilename));
                return false;
            }

            // Scale Delta Frame
            if ((boneHeader & 8) == 8) {
                state->scale = state->nextScale;
                state->scaleFrame = state->nextScaleFrame;

                qint16 nextFrame, x, y, z;
                stream >> nextFrame >> x >> y >> z;

                state->nextScaleFrame = nextFrame;
                state->nextScale.setX(x * scaleFactor);
                state->nextScale.setY(y * scaleFactor);
                state->nextScale.setZ(z * scaleFactor);
            }

            // Rotation Delta Frame
            if ((boneHeader & 4) == 4) {
                state->rotation = state->nextRotation;
                state->rotationFrame = state->nextRotationFrame;

                qint16 nextFrame, x, y, z, w;
                stream >> nextFrame >> x >> y >> z >> w;

                state->nextRotationFrame = nextFrame;
                state->nextRotation.setX(x * rotationFactor);
                state->nextRotation.setY(y * rotationFactor);
                state->nextRotation.setZ(z * rotationFactor);
                state->nextRotation.setScalar(w * rotationFactor);
            }

            // Translation Delta Frame
            if ((boneHeader & 2) == 2) {
                state->translation = state->nextTranslation;
                state->translationFrame = state->nextTranslationFrame;

                qint16 nextFrame, x, y, z;
                stream >> nextFrame >> x >> y >> z;

                state->nextTranslationFrame = nextFrame;
                state->nextTranslation.setX(x * translationFactor);
                state->nextTranslation.setY(y * translationFactor);
                state->nextTranslation.setZ(z * translationFactor);
            }

            // Read header of next chunk
            stream >> boneHeader;
        }

        // Store the header for the next key frame
        nextFrameId = boneHeader >> 1;
        return true;
    }

    void AnimationStream::rewind()
    {
        stream.device()->seek(_dataStart + 2 * sizeof(float)); // Seek to the start of the bone data

        qint16 boneId;
        qint16 x, y, z, w; // Vector components

        stream >> boneId;

        int i = 0;

        while (boneId != -2) {

            AnimationBoneState &boneState = _boneStates[i++];
            _boneMap[boneId] = &boneState;

            boneState.boneId = boneId;

            stream >> x >> y >> z; // Scale
            boneState.scale.setX(x * scaleFactor);
            boneState.scale.setY(y * scaleFactor);
            boneState.scale.setZ(z * scaleFactor);
            boneState.scaleFrame = 0;
            boneState.nextScaleFrame = 0;
            boneState.nextScale = boneState.scale;

            stream >> x >> y >> z >> w; // Rotation
            boneState.rotation.setVector(x * rotationFactor, y * rotationFactor, z * rotationFactor);
            boneState.rotation.setScalar(w * rotationFactor);
            boneState.rotationFrame = 0;
            boneState.nextRotationFrame = 0;
            boneState.nextRotation = boneState.rotation;

            stream >> x >> y >> z; // Translation
            boneState.translation.setX(x * translationFactor);
            boneState.translation.setY(y * translationFactor);
            boneState.translation.setZ(z * translationFactor);
            boneState.translationFrame = 0;
            boneState.nextTranslationFrame = 0;
            boneState.nextTranslation = boneState.translation;

            stream >> boneId;
        }

        if (!stream.atEnd()) {
            stream >> nextFrameId;

            if (nextFrameId == -2) {
                nextFrameId = -1; // No idea why this happens
            } else {
                readNextFrame();
            }
        } else {
            qWarning("Stream ended after initial bone list.");
        }
    }

    void AnimationStream::seek(int frame)
    {
        while (nextFrameId < frame && nextFrameId > 0) {
            readNextFrame();
        }
    }

}
