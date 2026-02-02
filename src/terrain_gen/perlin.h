#pragma once
#include <iostream>
#include <random>
#include <cmath>
#include <ctime>

#define M_PI 3.14159265

struct vector2
{
  float x;
  float y;
};

class Perlin
{
private:
  unsigned int seed;
  float hashToAngle(int x, int y);
  vector2 getGradient(int x, int y);
  float dotGridGradient(int ix, int iy, float x, float y);

public:
  Perlin();
  Perlin::Perlin(unsigned int seed);
  // ~Perlin();

  float getNoiseValue(float x, float y);
};