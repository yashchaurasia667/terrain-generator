#version 430 core

out vec4 FragColor;

// uniform sampler2D heightMap;
// in vec2 TexCoords;
in float Height;

void main() {
  float h = (Height + 16) / 64.0;
  FragColor = vec4(h, h, h, 1.0);
}
