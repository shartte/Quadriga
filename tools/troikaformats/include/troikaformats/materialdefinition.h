#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <QString>

#include "color.h"

class TroikaTextureStageInfo
{
public:
    /**
      Transforms used for texture coordinates.
      */
    enum UvType
    {
        Mesh = 0, // No texture matrix is used
        Drift, // Texture coordinate drifts linearily (Translation along one axis)
        Swirl, // Texture coordinates rotate
        Wavey, // Like drift but with a cosine/sine acceleration pattern
        Environment // Uses the surface normal
    };

    enum BlendType
    {
        Modulate,
        Add,
        TextureAlpha,
        CurrentAlpha,
        CurrentAlphaAdd
    };

    /**
      Constructs a new texture stage with the following default values:
      - blendType is set to Modulate
      - filename is null
      - uvType is Mesh
      - speedU is 1
      - speedV is 0
      */
    TroikaTextureStageInfo() : mBlendType(Modulate), mUvType(Mesh), mSpeedU(1), mSpeedV(0)
    {
    }

    /**
      Returns the blending type for multi-texturing that is applied to this texture stage.
      */
    BlendType blendType() const {
        return mBlendType;
    }

    void setBlendType(BlendType blendType) {
        mBlendType = blendType;
    }

    /**
      Returns the filename of the texture that is used in this stage.
      */
    const QString &filename() const {
        return mFilename;
    }

    void setFilename(const QString &filename) {
        mFilename = filename;
    }

    /**
      Returns the type of UV coordinates for this texture stage.
      */
    UvType uvType() const {
        return mUvType;
    }

    void setUvType(UvType uvType) {
        mUvType = uvType;
    }

    /**
      If the UV coordinates are animated, this returns the number of loops per minute
      for this texture's U coordinates.
      */
    float speedU() const {
        return mSpeedU;
    }

    void setSpeedU(float speed) {
        mSpeedU = speed;
    }

    /**
      If the UV coordinates are animated, this returns the number of loops per minute
      for this texture's V coordinates.
      */
    float speedV() const {
        return mSpeedV;
    }

    void setSpeedV(float speed) {
        mSpeedV = speed;
    }

private:
    BlendType mBlendType;
    QString mFilename;
    UvType mUvType;
    float mSpeedU, mSpeedV;
};

class TroikaMaterialDefinition
{
public:

    enum Type {
        UserDefined, // Through MDF files
        DepthArt, // Used by the depth geometry
        Placeholder // CHEST, etc.
    };

    enum BlendType {
        None,
        Alpha,
        Add,
        AlphaAdd
    };

    TroikaMaterialDefinition() :
            mDisableFaceCulling(false),
            mDisableLighting(false),
            mDisableDepthTest(false),
            mDisableDepthWrite(false),
            mLinearFiltering(false),
            mBlendType(Alpha),
            mRecalculateNormals(false),
            mSpecularPower(50),
            mColor(255, 255, 255, 255)
    {
    }

    static const int MaxTextureStages = 3;

    const QString &name() const
    {
        return mName;
    }

    Type type() const
    {
        return mType;
    }

    BlendType blendType() const
    {
        return mBlendType;
    }

    void setName(const QString &name)
    {
        mName = name;
    }

    void setType(Type type)
    {
        mType = type;
    }

    void setBlendType(BlendType blendType)
    {
        mBlendType = blendType;
    }

    bool isFaceCullingDisabled() const {
        return mDisableFaceCulling;
    }

    void setFaceCullingDisabled(bool faceCullingDisabled) {
        mDisableFaceCulling = faceCullingDisabled;
    }

    bool isLightingDisabled() const {
        return mDisableLighting;
    }

    void setLightingDisabled(bool lightingDisabled) {
        mDisableLighting = lightingDisabled;
    }

    bool isDepthTestDisabled() const {
        return mDisableDepthTest;
    }

    void setDepthTestDisabled(bool disabled) {
        mDisableDepthTest = disabled;
    }

    bool isLinearFiltering() const {
        return mLinearFiltering;
    }

    void setLinearFiltering(bool linearFiltering) {
        mLinearFiltering = linearFiltering;
    }

    bool isDepthWriteDisabled() const {
        return mDisableDepthWrite;
    }

    void setDepthWriteDisabled(bool depthWriteDisabled) {
        mDisableDepthWrite = depthWriteDisabled;
    }

    const TroikaColor &color() const {
        return mColor;
    }

    void setColor(const TroikaColor &color) {
        mColor = color;
    }

    const TroikaTextureStageInfo &textureStage(int stage) const {
        Q_ASSERT(stage < MaxTextureStages);
        return mTextureStages[stage];
    }

    TroikaTextureStageInfo &textureStage(int stage) {
        Q_ASSERT(stage < MaxTextureStages);
        return mTextureStages[stage];
    }

    float specularPower() const {
        return mSpecularPower;
    }

    void setSpecularPower(float specularPower) {
        mSpecularPower = specularPower;
    }

    const QString &glossMap() const {
        return mGlossMap;
    }

    void setGlossMap(const QString &glossMap) {
        mGlossMap = glossMap;
    }

    bool isRecalculateNormals() const {
        return mRecalculateNormals;
    }

    void setRecalculateNormals(bool recalculateNormals) {
        mRecalculateNormals = recalculateNormals;
    }

private:
    QString mName;
    Type mType;
    BlendType mBlendType;
    TroikaTextureStageInfo mTextureStages[MaxTextureStages];

    bool mDisableFaceCulling;
    bool mDisableLighting;
    bool mDisableDepthTest;
    bool mDisableDepthWrite;
    bool mLinearFiltering;
    bool mRecalculateNormals;
    float mSpecularPower;
    TroikaColor mColor;
    QString mGlossMap;
};

#endif // MATERIALDEFINITION_H
