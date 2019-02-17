// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "CoreMinimal.h"

class TERRAINGENERATOR_API TG_PerlinNoise
{
public:
  // Generate a new permutation vector based on the value of seed
  TG_PerlinNoise();

  void setNoiseSeed(const int32& newSeed);

  double noise(double x);
  double noise(double x, double y);
  double noise(double x, double y, double z);

  double octaveNoise(double x, int32 octaves);
  double octaveNoise(double x, double y, int32 octaves);
  double octaveNoise(double x, double y, double z, int32 octaves);

  double noise0_1(double x);
  double noise0_1(double x, double y);
  double noise0_1(double x, double y, double z);

  double octaveNoise0_1(double x, int32 octaves);
  double octaveNoise0_1(double x, double y, int32 octaves);
  double octaveNoise0_1(double x, double y, double z, int32 octaves);

private:
  TArray<int32> perm;
  double Fade(double t);
  double Lerp(double t, double a, double b);
  double Grad(int32 hash, double x, double y, double z);

};
