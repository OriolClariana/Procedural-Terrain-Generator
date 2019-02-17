// Procedural Terrain Generator by Oriol Marc Clariana Justes 2018 (https://oriolclariana.com)

#include "TG_PerlinNoise.h"

#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : (((int)x)-1) )

TG_PerlinNoise::TG_PerlinNoise()
{
}

void TG_PerlinNoise::setNoiseSeed(const int32& newSeed)
{
  // Set the Seed
  FMath::RandInit(newSeed);

  // Fill the array with default values
  perm.Init(0, 512);

  /* Shuffle the numbers */
  TArray<bool> availableSeeds;
  availableSeeds.Init(true, 256);
  for (int i = 0; i < 256; ++i) {
    // Generate one different number each time
    uint8 nextNumber;
    do {
      nextNumber = FMath::RandRange(0, 255);
    } while (!availableSeeds[nextNumber]);

    // Asign the number
    perm[i] = nextNumber;
    perm[i + 256] = nextNumber;
  }

}

double TG_PerlinNoise::Fade(double t)
{
  return t * t * t * (t * (t * 6 - 15) + 10);
}

double TG_PerlinNoise::Lerp(double t, double a, double b)
{
  return a + t * (b - a);
}

double TG_PerlinNoise::Grad(int32 hash, double x, double y, double z)
{
  const int32 h = hash & 15;
  const double u = h < 8 ? x : y;
  const double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double TG_PerlinNoise::noise(double x)
{
  return noise(x, 0.0, 0.0);
}

double TG_PerlinNoise::noise(double x, double y)
{
  return noise(x, y, 0.0);
}

double TG_PerlinNoise::noise(double x, double y, double z)
{
  const int32 X = FASTFLOOR(x) & 255;
  const int32 Y = FASTFLOOR(y) & 255;
  const int32 Z = FASTFLOOR(z) & 255;

  x -= FASTFLOOR(x);
  y -= FASTFLOOR(y);
  z -= FASTFLOOR(z);

  const double u = Fade(x);
  const double v = Fade(y);
  const double w = Fade(z);

  const int32 A = perm[X] + Y, AA = perm[A] + Z, AB = perm[A + 1] + Z;
  const int32 B = perm[X + 1] + Y, BA = perm[B] + Z, BB = perm[B + 1] + Z;

  return Lerp(w, Lerp(v, Lerp(u, Grad(perm[AA], x, y, z),
    Grad(perm[BA], x - 1, y, z)),
    Lerp(u, Grad(perm[AB], x, y - 1, z),
      Grad(perm[BB], x - 1, y - 1, z))),
    Lerp(v, Lerp(u, Grad(perm[AA + 1], x, y, z - 1),
      Grad(perm[BA + 1], x - 1, y, z - 1)),
      Lerp(u, Grad(perm[AB + 1], x, y - 1, z - 1),
        Grad(perm[BB + 1], x - 1, y - 1, z - 1))));
}

double TG_PerlinNoise::octaveNoise(double x, int32 octaves)
{
  double result = 0.0;
  double amp = 1.0;

  for (int32 i = 0; i < octaves; ++i)
  {
    result += noise(x) * amp;
    x *= 2.0;
    amp *= 0.5;
  }

  return result;
}

double TG_PerlinNoise::octaveNoise(double x, double y, int32 octaves)
{
  double result = 0.0;
  double amp = 1.0;

  for (int32 i = 0; i < octaves; ++i)
  {
    result += noise(x, y) * amp;
    x *= 2.0;
    y *= 2.0;
    amp *= 0.5;
  }

  return result;
}

double TG_PerlinNoise::octaveNoise(double x, double y, double z, int32 octaves)
{
  double result = 0.0;
  double amp = 1.0;

  for (int32 i = 0; i < octaves; ++i)
  {
    result += noise(x, y, z) * amp;
    x *= 2.0;
    y *= 2.0;
    z *= 2.0;
    amp *= 0.5;
  }

  return result;
}

double TG_PerlinNoise::noise0_1(double x)
{
  return noise(x) * 0.5 + 0.5;
}

double TG_PerlinNoise::noise0_1(double x, double y)
{
  return noise(x, y) * 0.5 + 0.5;
}

double TG_PerlinNoise::noise0_1(double x, double y, double z)
{
  return noise(x, y, z) * 0.5 + 0.5;
}

double TG_PerlinNoise::octaveNoise0_1(double x, int32 octaves)
{
  return octaveNoise(x, octaves) * 0.5 + 0.5;
}

double TG_PerlinNoise::octaveNoise0_1(double x, double y, int32 octaves)
{
  return octaveNoise(x, y, octaves) * 0.5 + 0.5;
}

double TG_PerlinNoise::octaveNoise0_1(double x, double y, double z, int32 octaves)
{
  return octaveNoise(x, y, z, octaves) * 0.5 + 0.5;
}