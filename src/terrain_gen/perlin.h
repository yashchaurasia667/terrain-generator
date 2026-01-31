#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>
#include <cmath>
#include <ctime>

#define M_PI 3.14159265


/*
-----------------------USAGE-------------------------
generate a random angle for every corner of every cell
generate a perlin noise value for very pixel on the screen
use the noise as a texture image or a bump map for terrain generation
*/

class Perlin
{
private:
  unsigned int scr_width, cell_width, num_cells, translator;

  glm::vec2 **vertex_angles;
  float **pixels;
  std::mt19937 rng;
  std::uniform_real_distribution<float> dist;
  // glm::vec2 vertex_angles[num_cells][num_cells];
  // float pixels[scr_width][scr_width];

  glm::vec2 generateRandomAngles();
  glm::vec2 translatePixelToCoordinate(int x, int y);
  float dotGridGradient(int ix, int iy, float x, float y);
  float calculate_perlin(float x, float y);

public:
  Perlin(int screen_width, int cell_width);
  ~Perlin();
};