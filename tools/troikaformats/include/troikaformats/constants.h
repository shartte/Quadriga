#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
  The number of pixels per world coordinate unit.
  */
const float PixelPerWorldTile = 28.2842703f;

const float Pi = 3.14159265358979323846f;

/**
  The original game applies an additional base rotation to everything in order to align it
  with the isometric grid. This is the radians value of that rotation.
  */
const float LegacyBaseRotation = 0.77539754f;

#endif // CONSTANTS_H
