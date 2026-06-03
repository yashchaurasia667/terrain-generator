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

void Terrain::initShader(const char *compute, int rezScale, const char *vert,
                         const char *frag, const char *geometry,
                         const char *tess_control,
                         const char *tess_evaluation) {
  noiseShader = ComputeShader(compute);
  shader = Shader(vert, frag, geometry, tess_control, tess_evaluation);

  unsigned int texResolution = rezScale * chunkWidth;
  glGenTextures(1, &noise_tex);
  glBindTexture(GL_TEXTURE_2D, noise_tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texResolution, texResolution, 0,
               GL_RGBA, GL_FLOAT, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  initTerrain(rezScale);
}

void Terrain::uploadVertexData() {
  vao.bind();
  vbo.bind();
  vbo.setData(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
  vao.addBuffer(vbo, layout);
}

void Terrain::generateVertices() {
  // std::vector<float> vertices;
  for (int i = 0; i < rez; i++) {
    for (int j = 0; j < rez; j++) {
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

void Terrain::initTerrain(int rezScale) {
  unsigned int texResolution = rezScale * chunkWidth;
  noiseShader.bind();
  noiseShader.setInt("u_seed", noiseSeed);
  noiseShader.setVec2("u_chunkOffset", glm::vec2(0.0f));
  noiseShader.setInt("u_cellWidth", cellWidth);
  noiseShader.setInt("u_chunkWidth", chunkWidth);
  noiseShader.setInt("u_noisePass", noisePass);
  // noiseShader.setFloat("u_amplitude", amp);
  noiseShader.setFloat("u_frequency", freq);
  noiseShader.setFloat("u_slopeStrength", slopeStrength);
  noiseShader.setFloat("u_lacunarity", lacunarity);
  noiseShader.setFloat("u_persistance", persistance);

  glBindImageTexture(0, noise_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
  noiseShader.setInt("u_heightMap", 0);
  glDispatchCompute((texResolution + 15) / 16, (texResolution + 15) / 16, 1);
  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Terrain::render(Camera camera, glm::mat4 model, glm::mat4 projection) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, noise_tex);

  shader.bind();
  shader.setMat4("model", model);
  shader.setMat4("view", camera.getViewMatrix());
  shader.setMat4("projection", projection);

  shader.setInt("heightMap", 0);
  shader.setInt("MIN_TESS_LEVEL", tess_min_level);
  shader.setInt("MAX_TESS_LEVEL", tess_max_level);
  shader.setFloat("MIN_DISTANCE", tess_min_dist);
  shader.setFloat("MAX_DISTANCE", tess_max_dist);

  shader.setFloat("u_amplitude", amp);
  shader.setFloat("u_chunkWidth", chunkWidth);
  shader.setInt("u_noisePass", noisePass);

  shader.setVec3("u_lightDir", glm::normalize(lightDir));
  shader.setVec3("u_lightColor", glm::normalize(lightColor));
  shader.setVec3("u_viewPos", camera.getPos());

  shader.setFloat("u_texScale", texScale);
  shader.setInt("u_normalMap", 1);
  shader.setVec3("u_terrainColor", terrainColor);

  shader.setVec3("u_snowColor", snowColor);
  shader.setFloat("u_snowSlopeMax", snowSlopeMax);
  shader.setFloat("u_snowSlopeMin", snowSlopeMin);

  vao.bind();
  glDrawArrays(GL_PATCHES, 0, rez * rez * 4);
}
