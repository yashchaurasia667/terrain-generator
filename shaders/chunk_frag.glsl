#version 430 core
out vec4 FragColor;

uniform sampler2D u_normalMap;
uniform float u_amplitude;
uniform vec3 u_lightColor;

in VS_OUT {
  vec2 TexCoords;
  float Height;
  vec3 Normal;
  vec3 TangentFragPos;
  vec3 TangetLightDir;
  vec3 TangetViewPos;
} fs_in;

void main() {
  vec3 norm = texture(u_normalMap, fs_in.TexCoords).rgb * 2.0 - 1.0;
  norm = normalize(norm);

  // vec3 norm = normalize(fs_in.Normal);
  vec3 lightDir = normalize(fs_in.TangetLightDir);
  vec3 viewDir = normalize(fs_in.TangetViewPos - fs_in.TangentFragPos);
  vec3 halfDir = normalize(lightDir + viewDir);

  // constant ambient
  float ambient = 0.15;

  // half-lambertian diffuse: maps [-1,1] dot product to [0,1]
  float NdotL = dot(norm, lightDir);
  float diffuse = NdotL * 0.5 + 0.5; // half-lambert
  diffuse *= diffuse; // square for more contrast

  // blinn-phong specular
  float spec = pow(max(dot(norm, halfDir), 0.0), 64.0);
  float specular = spec * 0.3;

  vec3 baseColor = vec3(0.35, 0.28, 0.15);

  vec3 result = baseColor * (ambient + diffuse * u_lightColor) + u_lightColor * specular;
  FragColor = vec4(result, 1.0);
}
