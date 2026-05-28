#version 430 core
layout(quads, fractional_odd_spacing, ccw) in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap;
uniform float u_amplitude;

in vec2 TextureCoords[];

out vec2 TexCoords;
out float Height;
out vec3 FragPos;
out vec3 Normal;

void main() {
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  // bilinear interpolation of UVs
  vec2 t0 = (TextureCoords[1] - TextureCoords[0]) * u + TextureCoords[0];
  vec2 t1 = (TextureCoords[3] - TextureCoords[2]) * u + TextureCoords[2];
  TexCoords = (t1 - t0) * v + t0; // no vec2 type here — assigns to out variable

  // sample height and apply amplitude
  Height = texture(heightMap, TexCoords).r - u_amplitude;

  // finite difference normal from heightmap
  vec2 texelSize = vec2(1.0) / vec2(textureSize(heightMap, 0));
  float hL = texture(heightMap, TexCoords - vec2(texelSize.x, 0.0)).r * u_amplitude;
  float hR = texture(heightMap, TexCoords + vec2(texelSize.x, 0.0)).r * u_amplitude;
  float hD = texture(heightMap, TexCoords - vec2(0.0, texelSize.y)).r * u_amplitude;
  float hU = texture(heightMap, TexCoords + vec2(0.0, texelSize.y)).r * u_amplitude;
  // the 2.0 in Y controls normal smoothness — increase to flatten normals
  Normal = normalize(vec3(hL - hR, 2.0, hD - hU));

  // bilinear interpolation of position
  vec4 p0 = (gl_in[1].gl_Position - gl_in[0].gl_Position) * u + gl_in[0].gl_Position;
  vec4 p1 = (gl_in[3].gl_Position - gl_in[2].gl_Position) * u + gl_in[2].gl_Position;
  vec4 p = (p1 - p0) * v + p0;

  // displace along Y
  p.y += Height;

  FragPos = vec3(model * p);
  gl_Position = projection * view * model * p;
}
