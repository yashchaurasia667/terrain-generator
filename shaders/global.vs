#version 330 core

#define MAX_POINT_LIGHTS 64
#define MAX_SPOT_LIGHTS 8
#define MAX_DIFFUSE_TEXTURES 4
#define MAX_SPECULAR_TEXTURES 4

struct PointLight {
  vec3 position;
  vec3 color;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float strength;
  float constant;
  float linear;
  float quadratic;
};

struct DirectionalLight {
  vec3 direction;
  vec3 color;

  float strength;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct SpotLight {
  vec3 position;
  vec3 direction;
  vec3 color;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  float strength;
  float cutOff;
  float outerCutOff;
};

layout (std140) uniform lights {
  DirectionalLight directionalLight;
  int numPointLights;
  PointLight pointLights[MAX_POINT_LIGHTS];
  int numSpotLights;
  SpotLight spotlights[MAX_SPOT_LIGHTS];
};

layout (std140) uniform matrices {
  mat4 view;
  mat4 projection;
};
