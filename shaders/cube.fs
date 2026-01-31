#version 330 core

#define MAX_POINT_LIGHTS 64
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_SPOT_LIGHTS 8
#define MAX_DIFFUSE_TEXTURES 4
#define MAX_SPECULAR_TEXTURES 4

struct Material {
  int numDiffuseTextures;
  sampler2D diffuse[MAX_DIFFUSE_TEXTURES];
  int numSpecularTextures;
  sampler2D specular[MAX_SPECULAR_TEXTURES];
  float shininess;
};

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

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 cubeColor;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform Material material;

uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numDirectionalLights;
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform int numSpotLights;
uniform SpotLight spotlights[MAX_SPOT_LIGHTS];

out vec4 FragColor;

vec3 calculateDirectionalLighting(DirectionalLight light, vec3 norm, vec3 viewDir, vec3 diffuse_tex, vec3 specular_tex);
vec3 calculatePointLighting(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse_tex, vec3 specular_tex);
vec3 calculateSpotLighting(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse_tex, vec3 specular_tex);

void main()
{    
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);
  vec4 diffuse_tex = texture(material.diffuse[0], TexCoords);
  vec4 specular_tex = texture(material.specular[0], TexCoords);

  for(int i=1; i<material.numDiffuseTextures; i++)
    diffuse_tex = mix(texture(material.diffuse[i], TexCoords), diffuse_tex, 0.5);
  for(int i=1; i<material.numSpecularTextures; i++)
    specular_tex = mix(texture(material.specular[i], TexCoords), specular_tex, 0.5);

  vec3 result = vec3(0.0);

  for(int i=0; i < numPointLights; i++)
    result += calculatePointLighting(pointLights[i], norm, FragPos, viewDir, vec3(diffuse_tex), vec3(specular_tex));

  for(int i=0; i < numDirectionalLights; i++)
    result += calculateDirectionalLighting(directionalLights[i], norm, viewDir, vec3(diffuse_tex), vec3(specular_tex));

  for(int i=0; i < numSpotLights; i++)
    result += calculateSpotLighting(spotlights[i], norm, FragPos, viewDir, vec3(diffuse_tex), vec3(specular_tex));

  FragColor = vec4(result, 0.0);
}

vec3 calculateDirectionalLighting(DirectionalLight light, vec3 norm, vec3 viewDir, vec3 diffuse_tex, vec3 specular_tex) {
  vec3 lightDir = normalize(-light.direction);
  float diff = max(dot(norm, lightDir), 0);

  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0), material.shininess);
  // vec3 tex = vec3(texture(material.diffuse, TexCoords));

  vec3 ambient = light.ambient * diffuse_tex;
  vec3 diffuse = light.diffuse * diff * diffuse_tex;
  vec3 specular = light.specular * spec * specular_tex;

  vec3 result = ambient + diffuse + specular;
  result *= light.strength;
  result *= light.color;
  return result;
}

vec3 calculatePointLighting(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse_tex, vec3 specular_tex) {
  vec3 lightDir = normalize(light.position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
  // vec3 tex = vec3(texture(material.diffuse, TexCoords));

  vec3 ambient = light.ambient * diffuse_tex;
  vec3 diffuse = light.diffuse * diff * diffuse_tex;
  vec3 specular = light.specular * spec * specular_tex;
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  vec3 result = ambient + diffuse + specular;
  result *= light.strength;
  result *= light.color;
  return result;
}

vec3 calculateSpotLighting(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse_tex, vec3 specular_tex)
{
  vec3 lightDir = normalize(light.position - fragPos);
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
  
  vec3 ambient = light.ambient * diffuse_tex;

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * diffuse_tex;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * specular_tex;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0/(light.constant + light.linear * distance + light.quadratic * (distance * distance));

  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;

  vec3 result = ambient + diffuse + specular;
  result *= light.strength;
  result *= light.color;
  return result;
}