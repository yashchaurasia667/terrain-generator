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
  vec3 specular;            // 64 -> 80
  float strength;           // 80 -> 84 the whole struct needs to be padded to a multiple of 16 so 96 bytes total
};

struct SpotLight {
  vec3 position;            // 0 -> (12 + 4)
  vec3 direction;           // 16 -> 32
  vec3 color;               // 32 -> 48

  vec3 ambient;             // 48 -> 64
  vec3 diffuse;             // 64 -> 80
  vec3 specular;            // 80 -> 96

  float constant;           // 96 -> 100
  float linear;             // 100 -> 104
  float quadratic;          // 104 -> 108

  float strength;           // 108 -> 112
  float cutOff;             // 112 -> 116
  float outerCutOff;        // 116 -> 120 -> next multiple of 16 128
};

layout (std140) uniform lights {
  DirectionalLight directionalLight;              // 0 -> 96
  int numPointLights;                             // 96 -> 100 + 12 padding
  PointLight pointLights[MAX_POINT_LIGHTS];       // 112 -> (64 * 96) + 112
  int numSpotLights;                              // 6256-> 6260 + 12 padding
  SpotLight spotlights[MAX_SPOT_LIGHTS];          // 6272-> (8 * 128) + 6272
};
