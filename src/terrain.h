#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <camera.h>
#include <shader.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

#include <vector>

struct Chunk {
  glm::ivec2 offset;
  unsigned int heightMap = 0;
  bool ready = false;
};

class Terrain {
public:
  // terrain vars
  unsigned int chunkWidth, cellWidth, rez = 20, drawDist = 2;
  int tess_min_level = 4, tess_max_level = 64;
  float tess_min_dist = 10, tess_max_dist = 800;
  float texScale = 15.5f, slopeStrength = 1.2f, snowSlopeMax = 0.3f,
        snowSlopeMin = 1.2f;

  // noise vars
  int noiseSeed = 5;
  unsigned int noisePass = 10;
  float amp = 128.0f, freq = 0.29f, persistance = 0.43f, lacunarity = 2.7f;

  // lighting/color vars
  glm::vec3 lightDir = glm::vec3(0.6f, 1.0f, 0.4f),
            lightColor = glm::vec3(0.47f, 0.34f, 0.26f),
            ambient = glm::vec3(0.62f, 0.59f, 1.0f),
            terrainColor = glm::vec3(0.35, 0.28, 0.15),
            snowColor = glm::vec3(1.0);

  // shaders
  Shader shader;
  ComputeShader noiseShader;

  Terrain(unsigned int chunkWidth = 1000, unsigned int cellWidth = 200,
          unsigned int rez = 20, unsigned int drawDist = 2,
          unsigned int noisePass = 10, int noiseSeed = 5, float amp = 128.0f,
          float freq = 0.29f, float persistance = 0.43f,
          float lacunarity = 2.7f);
  ~Terrain();

  void initShader(const char *compute, float rezScale, const char *vert,
                  const char *frag, const char *geometry = nullptr,
                  const char *tess_control = nullptr,
                  const char *tess_evaluation = nullptr);
  void generateVertices();
  void uploadVertexData();
  void initializeTerrain(unsigned int texResolution);
  void render(Camera camera, glm::mat4 model, glm::mat4 projection);

private:
  std::vector<float> vertices;
  VertexArray vao;
  VertexBuffer vbo;
  VertexBufferLayout layout;
  std::vector<struct Chunk> chunks;
};
