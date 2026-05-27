#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;

uniform int u_seed;
uniform vec2 u_chunkOffset;
uniform writeonly image2D u_heightMap;

uniform int u_cellWidth;
uniform int u_chunkWidth;

vec3 directions[] = {
    vec3(1, 1, 0),
    vec3(-1, 1, 0),
    vec3(1, -1, 0),
    vec3(-1, -1, 0),
    vec3(1, 0, 1),
    vec3(-1, 0, 1),
    vec3(1, 0, -1),
    vec3(-1, 0, -1),
    vec3(0, 1, 1),
    vec3(0, 1, -1),
    vec3(0, -1, 1),
    vec3(0, -1, -1),
    vec3(1, 1, 0),
    vec3(-1, 1, 0),
    vec3(1, -1, 0),
    vec3(-1, -1, 0),
  };

float perlin(vec2 pos);
float cerp(float v0, float v1, float t);
float lerp(float v0, float v1, float t);
int pickGradient(int x, int y);

void main() {
  ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
  if (texCoord.x >= u_chunkWidth || texCoord.y >= u_chunkWidth)
    return;

  vec2 worldPos = u_chunkOffset + vec2(texCoord);
  float height = perlin(worldPos);
  imageStore(u_heightMap, texCoord, vec4(height, height, height, 1.0));
}

float lerp(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}

float cerp(float v0, float v1, float t) {
  return (v1 - v0) * (3.0f - t * 2.0f) * t * t + v0;
}

int pickGradient(int x, int y) {
  return ((x * 1836311903) ^ (y * 1971215073) ^ u_seed) & 15;
}

float perlin(vec2 pos) {
  // borders
  int x0 = int(floor(pos.x));
  int y0 = int(floor(pos.y));
  int x1 = x0 + u_cellWidth;
  int y1 = y0 + u_cellWidth;

  vec3 d00 = directions[pickGradient(x0, y0)];
  vec3 d01 = directions[pickGradient(x0, y1)];
  vec3 d10 = directions[pickGradient(x1, y0)];
  vec3 d11 = directions[pickGradient(x1, y1)];

  float nx = (pos.x - x0) / u_cellWidth;
  float ny = (pos.y - y0) / u_cellWidth;

  vec3 p00 = vec3(pos - vec2(x0, y0), 0.0);
  vec3 p01 = vec3(pos - vec2(x0, y1), 0.0);
  vec3 p10 = vec3(pos - vec2(x1, y0), 0.0);
  vec3 p11 = vec3(pos - vec2(x1, y1), 0.0);

  float n00 = dot(d00, p00);
  float n01 = dot(d01, p01);
  float n10 = dot(d10, p10);
  float n11 = dot(d11, p11);

  float ix0 = cerp(n00, n10, nx);
  float ix1 = cerp(n01, n11, nx);

  float iy = cerp(ix0, ix1, ny);
  return (iy + 1) / 2.0;
}
