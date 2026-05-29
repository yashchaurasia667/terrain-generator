#version 430 core
out vec4 FragColor;

uniform sampler2D u_normalMap;
uniform float u_amplitude;
uniform vec3 u_lightColor;
uniform float u_chunkWidth;
uniform float u_texScale;

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
  vec3 tangentNormal = texture(u_normalMap, tiledUV).rgb;
  tangentNormal = normalize(tangentNormal * 2.0 - 1.0);

  vec3 lightDir = normalize(fs_in.TangetLightDir);
  vec3 viewDir = normalize(fs_in.TangetViewPos - fs_in.TangentFragPos);
  vec3 halfDir = normalize(lightDir + viewDir);

  float ambient = 0.15;
  float NdotL = dot(tangentNormal, lightDir);
  float diffuse = NdotL * 0.5 + 0.5; // half-lambert
  diffuse *= diffuse; // square for more contrast

  // Use tangentNormal here as well!
  float spec = pow(max(dot(tangentNormal, halfDir), 0.0), 64.0);
  float specular = spec * 0.3;

  vec3 baseColor = vec3(0.35, 0.28, 0.15);

  vec3 result = baseColor * (ambient + diffuse * u_lightColor) + u_lightColor * specular;
  FragColor = vec4(result, 1.0);
}
