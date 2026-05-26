#version 430 core

out vec4 FragColor;

uniform sampler2D heightMap;
in vec2 TexCoords;

void main() {
  FragColor = texture(heightMap, TexCoords);
}
