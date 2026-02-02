#include "perlin.h"

float cubic_interpolate(float a0, float a1, float w)
{
  return (a1 - a0) * (3.0f - w * 2.0f) * w * w + a0;
}

float Perlin::hashToAngle(int x, int y)
{
  unsigned int h = seed;
  h ^= x * 374761393U;
  h ^= y * 668265263U;
  h = (h ^ (h >> 13)) * 1274126177U;
  h ^= (h >> 16);

  return (h & 0xFFFFFF) * (2.0f * M_PI / 16777216.0f);
}

vector2 Perlin::getGradient(int x, int y)
{
  float angle = hashToAngle(x, y);
  return vector2{sin(angle), cos(angle)};
}

float Perlin::dotGridGradient(int ix, int iy, float x, float y)
{
  vector2 gradient = getGradient(ix, iy);
  float dx = x - (float)ix;
  float dy = y - (float)iy;
  return (dx * gradient.x + dy * gradient.y);
}

Perlin::Perlin()
{
  seed = std::random_device{}();
}

Perlin::Perlin(unsigned int seed)
{
  this->seed = seed;
}

float Perlin::getNoiseValue(float x, float y)
{
  int x0 = (floor)x;
  int y0 = (floor)y;
  int x1 = x0 + 1;
  int y1 = y0 + 1;

  float nx = x - (float)x0;
  float ny = y - (float)y0;

  float n00 = dotGridGradient(x0, y0, x, y);
  float n10 = dotGridGradient(x1, y0, x, y);
  float n01 = dotGridGradient(x0, y1, x, y);
  float n11 = dotGridGradient(x1, y1, x, y);

  float ix0 = cubic_interpolate(n00, n10, nx);
  float ix1 = cubic_interpolate(n01, n11, nx);
  float value = cubic_interpolate(ix0, ix1, ny);

  return value;
}
