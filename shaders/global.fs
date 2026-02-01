#version 330 core

#define MAX_POINT_LIGHTS 64
#define MAX_SPOT_LIGHTS 8
#define MAX_DIFFUSE_TEXTURES 4
#define MAX_SPECULAR_TEXTURES 4

struct PointLight {
  vec3 position;            // 0 -> (12 + 4)
  vec3 color;               // 16 -> 32

  vec3 ambient;             // 32 -> 48
  vec3 diffuse;             // 48 -> 64
  vec3 specular;            // 64 -> 80

  float strength;           // 80 -> 84
  float constant;           // 84 -> 88
  float linear;             // 88 -> 92
  float quadratic;          // 92 -> 96
};

struct DirectionalLight {
  vec3 direction;           // 0 -> (12 + 4) including padding 4 bytes
  vec3 color;               // 16 -> 32

  vec3 ambient;             // 32 -> 48
  vec3 diffuse;             // 48 -> 64
  vec3 specular;            // 64 -> 76 no need for padding as alignment of float is 4 bytes
  float strength;           // 76 -> 80
};

struct SpotLight {
  vec3 position;            // 0 -> (12 + 4)
  vec3 direction;           // 16 -> 32
  vec3 color;               // 32 -> 48

  vec3 ambient;             // 48 -> 64
  vec3 diffuse;             // 64 -> 80
  vec3 specular;            // 80 -> 92

  float constant;           // 92 -> 96
  float linear;             // 96 -> 100
  float quadratic;          // 100 -> 104

  float strength;           // 104 -> 108
  float cutOff;             // 108 -> 112
  float outerCutOff;        // 112 -> 116 -> next multiple of 16 128
};

layout (std140) uniform lights {
  DirectionalLight directionalLight;              // 0 -> 80
  PointLight pointLights[MAX_POINT_LIGHTS];       // 80 -> (64 * 96) + 80
  SpotLight spotlights[MAX_SPOT_LIGHTS];          // 6224 -> (8 * 128) + 6224
  int numPointLights;                             // 7248 -> 7252
  int numSpotLights;                              // 7252 -> 7256
};
