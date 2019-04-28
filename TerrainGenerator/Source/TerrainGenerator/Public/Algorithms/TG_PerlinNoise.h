// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#pragma once

#include "CoreMinimal.h"

class TERRAINGENERATOR_API TG_PerlinNoise
{
public:
  // Generate a new permutation vector based on the value of seed
  TG_PerlinNoise();

  void setNoiseSeed(const int32& newSeed);

  double noise(double x = 0.0, double y = 0.0, double z = 0.0);
  double noise0_1(double x = 0.0, double y = 0.0, double z = 0.0);
  double octaveNoise(double x = 0.0, double y = 0.0, double z = 0.0, int32 octaves = 1);
  double octaveNoise0_1(double x = 0.0, double y = 0.0, double z = 0.0, int32 octaves = 1);

private:
  TArray<int32> perm;
  double Fade(double t);
  double Lerp(double t, double a, double b);
  double Grad(int32 hash, double x, double y, double z);

};
