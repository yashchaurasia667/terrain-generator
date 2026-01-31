#include "perlin.h"

float cubic_interpolate(float a0, float a1, float w)
{
  return (a1 - a0) * (3.0f - w * 2.0f) * w * w + a0;
}

Perlin::Perlin(int screen_width, int cell_width) : rng(std::random_device{}()), dist(0.0f, 2.0f * M_PI)
{
  scr_width = screen_width;
  cell_width = cell_width;
  num_cells = scr_width / cell_width;
  translator = scr_width / num_cells;

  vertex_angles = new glm::vec2 *[num_cells];
  for (unsigned int i = 0; i < num_cells; i++)
    vertex_angles[i] = new glm::vec2[num_cells];

  pixels = new float *[scr_width];
  for (unsigned int i = 0; i < scr_width; i++)
    pixels[i] = new float[scr_width];
}

glm::vec2 Perlin::generateRandomAngles()
{
  float angle = dist(rng);
  return glm::vec2((float)sin(angle), (float)cos(angle));
}

glm::vec2 Perlin::translatePixelToCoordinate(int x, int y)
{
  glm::vec2 coord;
  coord.x = (float)x / (float)translator;
  coord.y = (float)y / (float)translator;
  return coord;
}

float Perlin::dotGridGradient(int ix, int iy, float x, float y)
{
  glm::vec2 gradient = vertex_angles[ix][iy];
  float dx = x - (float)ix;
  float dy = y - (float)iy;
  return (dx * gradient.x + dy * gradient.y);
}

float Perlin::calculate_perlin(float x, float y)
{
  int x0 = (int)x;
  int y0 = (int)y;
  int x1 = x0 + 1;
  int y1 = y0 + 1;

  float nx = x - (float)x0;
  float ny = y - (float)y0;

  float n00 = dotGridGradient(x0, y0, x, y);
  float n10 = dotGridGradient(x1, y0, x, y);
  float ix0 = cubic_interpolate(n00, n10, nx);

  float n01 = dotGridGradient(x0, y1, x, y);
  float n11 = dotGridGradient(x1, y1, x, y);
  float ix1 = cubic_interpolate(n01, n11, nx);

  float value = cubic_interpolate(ix0, ix1, ny);
  return value;
}