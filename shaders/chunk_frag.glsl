#version 430 core
out vec4 FragColor;

uniform sampler2D heightMap;
uniform sampler2D u_terrainNormal;
uniform sampler2D u_waterNormal;
uniform float u_amplitude;

uniform float u_chunkWidth;
uniform vec3 u_terrainColor;
uniform float u_texScale;
uniform vec3 u_waterColor;

uniform vec3 u_lightColor;
uniform vec3 u_ambientColor;

uniform vec3 u_snowColor;
uniform float u_snowSlopeMax;
uniform float u_snowSlopeMin;

in VS_OUT {
  vec2 TexCoords;
  float Height;
  vec3 Normal;
  vec3 TangentFragPos;
  vec3 TangetLightDir;
  vec3 TangetViewPos;
} fs_in;

void main() {
  vec2 tiledUV = fs_in.TexCoords * (u_chunkWidth / u_texScale);

  vec3 tangentNormal;
  if (fs_in.Height > 0)
    tangentNormal = texture(u_terrainNormal, tiledUV).rgb;
  else
    tangentNormal = texture(u_waterNormal, tiledUV).rgb;
  tangentNormal = normalize(tangentNormal * 2.0 - 1.0);

  vec3 lightDir = normalize(fs_in.TangetLightDir);
  vec3 viewDir = normalize(fs_in.TangetViewPos - fs_in.TangentFragPos);
  vec3 halfDir = normalize(lightDir + viewDir);

  vec3 ambient = 0.15 * u_ambientColor;
  float NdotL = dot(tangentNormal, lightDir);
  float diffuse = NdotL * 0.5 + 0.5; // half-lambert
  diffuse *= diffuse; // square for more contrast

  // Use tangentNormal here as well!
  // float shininess = 256.0;
  // if(fs_in.Height > 0)
  //   shininess = 4.0;
  // float spec = pow(max(dot(tangentNormal, halfDir), 0.0), shininess);
  // float specular = spec * 0.3;
  float shininess = 256.0;
  float specularStrength = 0.3;

  if (fs_in.Height > 0) {
    shininess = 4.0;
    specularStrength = 0.3;
  } else {
    shininess = 1024.0; // much sharper point
    specularStrength = 3.0; // bright enough to blow out

    // Fresnel: specular intensifies at glancing angles
    float fresnel = pow(1.0 - max(dot(tangentNormal, viewDir), 0.0), 4.0);
    specularStrength = mix(0.8, 3.5, fresnel);
  }

  float spec = pow(max(dot(tangentNormal, halfDir), 0.0), shininess);
  float specular = spec * specularStrength;

  vec3 color = u_waterColor;
  if (fs_in.Height > 0) {
    vec2 gradient = texture(heightMap, fs_in.TexCoords).gb;
    float steepness = length(gradient);
    float snowSlope = smoothstep(u_snowSlopeMax, u_snowSlopeMin, steepness);

    float heightThreashold = -u_amplitude * 0.2;
    float heightFade = smoothstep(
        heightThreashold - u_amplitude * 0.15,
        heightThreashold + u_amplitude * 0.15,
        fs_in.Height
      );

    float snowAmount = snowSlope * heightFade;
    if (fs_in.Height == 0) snowAmount = 0;

    color = mix(u_terrainColor, u_snowColor, snowAmount);
  }

  vec3 result = color * (ambient + diffuse * u_lightColor) + u_lightColor * specular;
  FragColor = vec4(result, 1.0);
}
