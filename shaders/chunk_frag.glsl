#version 430 core

out vec4 FragColor;

uniform float u_amplitude;
// uniform sampler2D heightMap;
// in vec2 TexCoords;
in float Height;

void main() {
  float h = Height / u_amplitude;
  FragColor = vec4(h, h, h, 1.0);
}
