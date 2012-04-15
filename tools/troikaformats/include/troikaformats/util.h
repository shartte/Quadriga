#ifndef TROIKAFORMATS_UTIL_H
#define TROIKAFORMATS_UTIL_H

#include "constants.h"

class Box3D {
public:
    Box3D() {}

    Box3D(const QVector3D &minCorner, const QVector3D &maxCorner)
        : mMinCorner(minCorner), mMaxCorner(maxCorner)
    {}

    const QVector3D &minimum() const {
        return mMinCorner;
    }

    const QVector3D &maximum() const {
        return mMaxCorner;
    }

    void setExtents(const QVector3D &minCorner, const QVector3D &maxCorner)
    {
        mMinCorner = minCorner;
        mMaxCorner = maxCorner;
    }

private:
    QVector3D mMinCorner;
    QVector3D mMaxCorner;
};

/**
  Converts radians to degree.
  */
inline float rad2deg(float rad)
{
    return rad * 180.f / Pi;
}

#endif // UTIL_H
