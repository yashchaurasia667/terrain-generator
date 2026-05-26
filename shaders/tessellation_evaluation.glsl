#version 430 core

layout(quads, fractional_odd_spacing, ccw) in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec2 TextureCoords[];
out vec2 TexCoords;
out float Height;

void main()
{
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;
  Height = 0.0;

  vec4 p00 = gl_in[0].gl_Position;
  vec4 p01 = gl_in[1].gl_Position;
  vec4 p10 = gl_in[2].gl_Position;
  vec4 p11 = gl_in[3].gl_Position;

  vec4 p0 = (p01 - p00) * u + p00;
  vec4 p1 = (p11 - p10) * u + p10;
  vec4 p = (p1 - p0) * v + p0;

  vec2 t0 = (TextureCoords[1] - TextureCoords[0]) * u + TextureCoords[0];
  vec2 t1 = (TextureCoords[3] - TextureCoords[2]) * u + TextureCoords[2];
  vec2 TexCoords = (t1 - t0) * v + t0;

  gl_Position = projection * view * model * p;
}
