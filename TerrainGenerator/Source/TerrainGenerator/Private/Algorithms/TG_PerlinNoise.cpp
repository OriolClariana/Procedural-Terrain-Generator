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

double TG_PerlinNoise::noise(double x, double y, double z)
{
  const auto unit_x = (int)floor(x) & 255,
    unit_y = (int)floor(y) & 255,
    unit_z = (int)floor(z) & 255;
  const auto sub_x = x - floor(x),
    sub_y = y - floor(y),
    sub_z = z - floor(z);
  const auto  u = Fade(sub_x),
    v = Fade(sub_y),
    w = Fade(sub_z);
  const auto a = perm[unit_x] + unit_y,
    aa = perm[a] + unit_z,
    ab = perm[a + 1] + unit_z,
    b = perm[unit_x + 1] + unit_y,
    ba = perm[b] + unit_z,
    bb = perm[b + 1] + unit_z;

  return Lerp(w, Lerp( v, Lerp( u, Grad( perm[aa], sub_x, sub_y, sub_z ),
    Grad( perm[ba], sub_x - 1, sub_y, sub_z ) ),
    Lerp( u, Grad( perm[ab], sub_x, sub_y - 1, sub_z ),
      Grad( perm[bb], sub_x - 1, sub_y - 1, sub_z ) ) ),
    Lerp( v, Lerp( u,
        Grad( perm[aa + 1], sub_x, sub_y, sub_z - 1 ),
        Grad( perm[ba + 1], sub_x - 1, sub_y, sub_z - 1 ) ),
      Lerp( u,
        Grad( perm[ab + 1], sub_x, sub_y - 1, sub_z - 1 ),
        Grad( perm[bb + 1], sub_x - 1, sub_y - 1, sub_z - 1 ) ) ) );
}

double TG_PerlinNoise::noise0_1(double x, double y, double z)
{
  return noise(x, y, z) * 0.5 + 0.5;
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

double TG_PerlinNoise::octaveNoise0_1(double x, double y, double z, int32 octaves)
{
  return octaveNoise(x, y, z, octaves) * 0.5 + 0.5;
}