#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader.h>
#include <camera.h>
#include <glm/gtc/matrix_transform.hpp>

#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

#include <vector>

class Terrain {
public:
  int chunkWidth, cellWidth, noiseSeed;
  float tess_min_dist = 10, tess_max_dist = 800;
  int tess_min_level = 4, tess_max_level = 64;
  unsigned int rez = 20;
  Shader shader;

  Terrain(int chunkWidth = 1000, int cellWidth = 200, int noiseSeed = 0,
          unsigned int rez = 20);
  ~Terrain();

  void initShader(const char *vert, const char *frag,
                  const char *geometry = nullptr,
                  const char *tess_control = nullptr,
                  const char *tess_evaluation = nullptr);
  void generateVertices();
  void uploadVertexData();
  void render(Camera camera, glm::mat4 model, glm::mat4 projection,
              int heightMap);

private:
  std::vector<float> vertices;
  VertexArray vao;
  VertexBuffer vbo;
  VertexBufferLayout layout;
};
