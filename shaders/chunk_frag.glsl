#version 430 core
out vec4 FragColor;

in float Height;
in vec3 FragPos;
in vec3 Normal;

uniform float u_amplitude;
uniform vec3 u_lightDir;    // normalized direction TOWARD the sun
uniform vec3 u_viewPos;     // camera world position

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_lightDir);
    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // constant ambient
    float ambient = 0.15;

    // half-lambertian diffuse: maps [-1,1] dot product to [0,1]
    float NdotL = dot(norm, lightDir);
    float diffuse = NdotL * 0.5 + 0.5;   // half-lambert
    diffuse *= diffuse;                    // square for more contrast

    // blinn-phong specular
    float spec = pow(max(dot(norm, halfDir), 0.0), 64.0);
    float specular = spec * 0.3;

    // height-based color blending
    float t = clamp(Height / u_amplitude, 0.0, 1.0);
    vec3 highColor = vec3(0.35, 0.28, 0.15);  // dirt/valley
    vec3 lowColor = vec3(0.55, 0.65, 0.35);  // grass/hill
    // vec3 baseColor = mix(lowColor, highColor, t);
    vec3 baseColor = vec3(0.35, 0.28, 0.15);

    vec3 lightColor = vec3(1.0, 0.95, 0.8);   // warm sunlight

    vec3 result = baseColor * (ambient + diffuse * lightColor) + lightColor * specular;
    FragColor = vec4(result, 1.0);
}
