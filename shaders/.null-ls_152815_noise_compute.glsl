#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;

uniform int u_seed;
uniform vec2 u_chunkOffset;
uniform writeonly image2D u_heightMap;

float perlin(int seed, vec2 worldPos);

void main() {
  ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
  vec2 worldPos = u_chunkOffset + vec2(texCoord);

  float height = perlin(u_seed, worldPos);
  imageStore(u_heightMap, texCoord, vec4(height));
}

float perlin(int seed, vec2 worldPos) {
}
