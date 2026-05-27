#include "terrain.h"

Terrain::Terrain(int chunkWidth, int cellWidth, int noiseSeed,
                 unsigned int rez) {
  this->chunkWidth = chunkWidth;
  this->cellWidth = cellWidth;
  this->noiseSeed = noiseSeed;
  this->rez = rez;
  layout.push<float>(3);
  layout.push<float>(2);
  generateVertices();
  uploadVertexData();
}

Terrain::~Terrain() {}

void Terrain::initShader(const char *vert, const char *frag,
                         const char *geometry, const char *tess_control,
                         const char *tess_evaluation) {
  shader = Shader(vert, frag, geometry, tess_control, tess_evaluation);
}

void Terrain::uploadVertexData() {
  vao.bind();
  vbo.bind();
  vbo.setData(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
  vao.addBuffer(vbo, layout);
}

void Terrain::generateVertices() {
  // std::vector<float> vertices;
  for (unsigned int i = 0; i < rez; i++) {
    for (unsigned int j = 0; j < rez; j++) {
      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * i) / (float)rez); // v.x
      vertices.push_back(0.0f);                          // v.y
      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * j) / (float)rez); // v.z
      vertices.push_back(i / (float)rez);                // u
      vertices.push_back(j / (float)rez);                // v

      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * (i + 1)) / (float)rez); // v.x
      vertices.push_back(0.0f);                                // v.y
      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * j) / (float)rez); // v.z
      vertices.push_back((i + 1) / (float)rez);          // u
      vertices.push_back(j / (float)rez);                // v

      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * i) / (float)rez); // v.x
      vertices.push_back(0.0f);                          // v.y
      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * (j + 1)) / (float)rez); // v.z
      vertices.push_back(i / (float)rez);                      // u
      vertices.push_back((j + 1) / (float)rez);                // v

      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * (i + 1)) / (float)rez); // v.x
      vertices.push_back(0.0f);                                // v.y
      vertices.push_back(-chunkWidth / 2.0f +
                         (chunkWidth * (j + 1)) / (float)rez); // v.z
      vertices.push_back((i + 1) / (float)rez);                // u
      vertices.push_back((j + 1) / (float)rez);                // v
    }
  }
  // return vertices;
}

void Terrain::render(Camera camera, glm::mat4 model, glm::mat4 projection,
                     int heightMap) {
  shader.bind();
  shader.setMat4("model", model);
  shader.setMat4("view", camera.getViewMatrix());
  shader.setMat4("projection", projection);

  shader.setInt("heightMap", heightMap);
  shader.setInt("MIN_TESS_LEVEL", tess_min_level);
  shader.setInt("MAX_TESS_LEVEL", tess_max_level);
  shader.setFloat("MIN_DISTANCE", tess_min_dist);
  shader.setFloat("MAX_DISTANCE", tess_max_dist);

  vao.bind();
  glDrawArrays(GL_PATCHES, 0, rez * rez * 4);
}
