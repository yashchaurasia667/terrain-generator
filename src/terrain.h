#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <camera.h>
#include <shader.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

class Terrain {
public:
  // terrain vars
  int chunkWidth, cellWidth, rez = 20, drawDist = 2;
  int tess_min_level = 4, tess_max_level = 64;
  float tess_min_dist = 10, tess_max_dist = 800;
  float texScale = 15.5f, slopeStrength = 1.2f, snowSlopeMax = 0.3f,
        snowSlopeMin = 1.2f;

  // noise vars
  int noiseSeed = 5;
  int noisePass = 10;
  float amp = 128.0f, freq = 0.29f, persistance = 0.43f, lacunarity = 2.7f;

  // lighting/color vars
  glm::vec3 lightDir = glm::vec3(0.6f, 1.0f, 0.4f),
            lightColor = glm::vec3(0.47f, 0.34f, 0.26f),
            ambient = glm::vec3(0.62f, 0.59f, 1.0f),
            terrainColor = glm::vec3(0.35, 0.28, 0.15),
            snowColor = glm::vec3(1.0);

  // shaders
  ComputeShader noiseShader;
  Shader shader;

  // temp
  unsigned int noise_tex = 0;

  Terrain(int chunkWidth = 1000, int cellWidth = 200, int noiseSeed = 0,
          unsigned int rez = 20);
  ~Terrain();

  void initShader(const char *compute, int rezScale, const char *vert,
                  const char *frag, const char *geometry = nullptr,
                  const char *tess_control = nullptr,
                  const char *tess_evaluation = nullptr);
  void generateVertices();
  void uploadVertexData();
  void initTerrain(int rezScale = 1);
  void render(Camera camera, glm::mat4 model, glm::mat4 projection);

private:
  std::vector<float> vertices;
  VertexArray vao;
  VertexBuffer vbo;
  VertexBufferLayout layout;
};
