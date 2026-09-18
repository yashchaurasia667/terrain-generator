#include "terrain.h"
#include <chrono>
#include <iostream>

Terrain::Terrain(int chunkWidth, int cellWidth, int noiseSeed, unsigned int rez,
                 int drawDist) {
  this->_chunk_width = chunkWidth;
  this->_cell_width = cellWidth;
  this->_noise_seed = noiseSeed;
  this->_rez = rez;
  this->_draw_dist = drawDist;

  size_t chunk_size = (_draw_dist * 2 - 1) * (_draw_dist * 2 - 1);
  _chunks.reserve(chunk_size);

  _layout.push<float>(3);
  _layout.push<float>(2);
  generateVertices();
  uploadVertexData();
  generateChunks();
}

Terrain::~Terrain() { _chunk_deletion_que.flush(); }

void Terrain::generateChunks() {
  _chunk_deletion_que.flush();
  _chunks.clear();

  // drawDist -> 1 to n
  unsigned int n = _draw_dist * 2 - 1;
  int half = (int)(n / 2);
  for (int cy = -half; cy <= half; cy++) {
    for (int cx = -half; cx <= half; cx++) {
      Chunk c;
      c.coord = glm::ivec2(cx, cy);
      c.ready = false;
      c.needsRegen = false;
      _chunks.push_back(c);
    }
  }
  auto start = std::chrono::high_resolution_clock::now();
  generateChunkTextures();
  auto end = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
  // std::cout << "genereate chunk textures took: " << ms << " ms" << std::endl;
}

void Terrain::generateChunkTextures() {
  for (unsigned int i = 0; i < _chunks.size(); i++) {
    glGenTextures(1, &_chunks[i].heightMap);
    glBindTexture(GL_TEXTURE_2D, _chunks[i].heightMap);
    glTexImage2D(GL_TEXTURE_2D, 0, heightMapType, heightMapResolution,
                 heightMapResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const unsigned int id = _chunks[i].heightMap;
    _chunk_deletion_que.push([id]() { glDeleteTextures(1, &id); });
  }
}

void Terrain::initShader(const char *compute, const char *vert,
                         const char *frag, const char *geometry,
                         const char *tess_control,
                         const char *tess_evaluation) {
  _noise_shader = ComputeShader(compute);
  _shader = Shader(vert, frag, geometry, tess_control, tess_evaluation);

  initTerrain();
}

void Terrain::initTerrain() {
  for (int i = 0; i < (int)_chunks.size(); i++) {

    auto start = std::chrono::high_resolution_clock::now();
    generateChunkHeightmap(i);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();
    // std::cout << i << " heightmap Took: " << ms << "ms" << std::endl;
  }
  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Terrain::generateChunkHeightmap(int idx) {
  Chunk &c = _chunks[idx];
  // world offset: chunk coord × chunk size in world units
  glm::vec2 worldOffset = glm::vec2(c.coord) * (float)heightMapResolution;

  _noise_shader.bind();
  _noise_shader.setInt("u_seed", _noise_seed);
  _noise_shader.setInt("u_cellWidth", _cell_width);
  _noise_shader.setInt("u_chunkWidth", _chunk_width);
  _noise_shader.setInt("u_noisePass", _noise_pass);
  _noise_shader.setFloat("u_amplitude", _amp);
  _noise_shader.setFloat("u_frequency", _freq);
  _noise_shader.setFloat("u_slopeStrength", _slope_strength);
  _noise_shader.setFloat("u_lacunarity", _lacunarity);
  _noise_shader.setFloat("u_persistance", _persistance);
  _noise_shader.setInt("u_heightMap", 0);
  _noise_shader.setVec2("u_chunkOffset", worldOffset);

  // GL_RGBA8
  glBindImageTexture(0, c.heightMap, 0, GL_FALSE, 0, GL_READ_WRITE,
                     heightMapType);
  glDispatchCompute((heightMapResolution + 15) / 16,
                    (heightMapResolution + 15) / 16, 1);
  // glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

  c.ready = true;
  c.needsRegen = false;
}

void Terrain::updateChunks(glm::vec3 playerPos) {
  glm::ivec2 playerChunk =
      glm::ivec2((int)std::round(playerPos.x / _chunk_width),
                 (int)std::round(playerPos.z / _chunk_width));

  if (playerChunk == _last_player_chunk)
    return;
  _last_player_chunk = playerChunk;

  unsigned int n = _draw_dist * 2 - 1;
  int half = (int)(n / 2);

  std::vector<size_t> outOfRangeIndices;
  outOfRangeIndices.reserve(_chunks.size());

  std::vector<bool> covered(n * n, false);

  for (size_t i = 0; i < _chunks.size(); i++) {
    glm::ivec2 diff = _chunks[i].coord - playerChunk;

    if (std::abs(diff.x) > half || std::abs(diff.y) > half) {
      outOfRangeIndices.push_back(i);
    } else {
      // Map the coordinate difference to our local grid (0 to n-1)
      int gridX = diff.x + half;
      int gridY = diff.y + half;
      covered[gridY * n + gridX] = true;
    }
  }

  // PASS 2: Find uncovered spots in our needed radius and recycle out-of-range
  // chunks to fill them
  size_t recycleIdx = 0;

  for (int cy = -half; cy <= half; cy++) {
    for (int cx = -half; cx <= half; cx++) {
      int gridX = cx + half;
      int gridY = cy + half;

      // If this spot isn't covered by an existing chunk
      if (!covered[gridY * n + gridX]) {

        // Safety guard: ensure we have a chunk to recycle
        if (recycleIdx >= outOfRangeIndices.size())
          break;

        // Get the index of an out-of-range chunk
        size_t chunkIdx = outOfRangeIndices[recycleIdx++];

        // Recycle it
        _chunks[chunkIdx].coord = playerChunk + glm::ivec2(cx, cy);
        _chunks[chunkIdx].ready = false;
        _chunks[chunkIdx].needsRegen = true;
        _regenQueue.push(chunkIdx);
      }
    }
  }
}

void Terrain::processRegenQueue() {
  if (_regenQueue.empty())
    return;

  int idx = _regenQueue.front();
  _regenQueue.pop();
  generateChunkHeightmap(idx);
  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void Terrain::uploadVertexData() {
  _vao.bind();
  _vbo.bind();
  _vbo.setData(_vertices.size() * sizeof(float), &_vertices[0], GL_STATIC_DRAW);
  _vao.addBuffer(_vbo, _layout);
}

void Terrain::generateVertices() {
  _vertices.clear();
  for (int i = 0; i < _rez; i++) {
    for (int j = 0; j < _rez; j++) {
      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * i) / (float)_rez);
      _vertices.push_back(0.0f);
      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * j) / (float)_rez);
      _vertices.push_back(i / (float)_rez);
      _vertices.push_back(j / (float)_rez);

      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * (i + 1)) / (float)_rez);
      _vertices.push_back(0.0f);
      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * j) / (float)_rez);
      _vertices.push_back((i + 1) / (float)_rez);
      _vertices.push_back(j / (float)_rez);

      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * i) / (float)_rez);
      _vertices.push_back(0.0f);
      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * (j + 1)) / (float)_rez);
      _vertices.push_back(i / (float)_rez);
      _vertices.push_back((j + 1) / (float)_rez);

      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * (i + 1)) / (float)_rez);
      _vertices.push_back(0.0f);
      _vertices.push_back(-_chunk_width / 2.0f +
                          (_chunk_width * (j + 1)) / (float)_rez);
      _vertices.push_back((i + 1) / (float)_rez);
      _vertices.push_back((j + 1) / (float)_rez);
    }
  }
}

void Terrain::render(Camera camera, glm::mat4 model, glm::mat4 projection) {
  updateChunks(camera.getPos());
  processRegenQueue();

  _shader.bind();

  _shader.setMat4("view", camera.getViewMatrix());
  _shader.setMat4("projection", projection);

  _shader.setInt("MIN_TESS_LEVEL", _tess_min_level);
  _shader.setInt("MAX_TESS_LEVEL", _tess_max_level);
  _shader.setFloat("MIN_DISTANCE", _tess_min_dist);
  _shader.setFloat("MAX_DISTANCE", _tess_max_dist);

  _shader.setFloat("u_amplitude", _amp);
  _shader.setFloat("u_chunkWidth", (float)_chunk_width);
  _shader.setInt("u_noisePass", _noise_pass);

  _shader.setVec3("u_lightDir", glm::normalize(_light_dirn));
  _shader.setVec3("u_lightColor", _light_color);
  _shader.setVec3("u_ambientColor", _ambient);
  _shader.setVec3("u_viewPos", camera.getPos());

  _shader.setFloat("u_texScale", _tex_scale);
  _shader.setVec3("u_terrainColor", _terrain_color);
  _shader.setVec3("u_waterColor", _water_color);
  _shader.setVec3("u_snowColor", _snow_color);
  _shader.setFloat("u_snowSlopeMax", _snow_slope_max);
  _shader.setFloat("u_snowSlopeMin", _snow_slope_min);

  viewFrustum.extract(projection * camera.getViewMatrix());
  _drawn_chunks = 0;
  _culled_chunks = 0;
  for (Chunk &c : _chunks) {
    if (!c.ready)
      continue;

    glm::vec2 worldPos = glm::vec2(c.coord) * (float)_chunk_width;
    glm::mat4 chunkModel = glm::translate(
        glm::mat4(1.0f), glm::vec3(worldPos.x, 0.0f, worldPos.y));

    float half = _chunk_width / 2.0f;

    glm::vec3 bmin(worldPos.x - half, -_amp * 0.1f, worldPos.y - half);
    glm::vec3 bmax(worldPos.x + half, _amp, worldPos.y + half);
    if (!viewFrustum.intersectsAABB(bmin, bmax)) {
      _culled_chunks++;
      continue;
    }
    _drawn_chunks++;

    if (!viewFrustum.intersectsAABB(bmin, bmax))
      continue; // CULLED

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, c.heightMap);

    _shader.setMat4("model", chunkModel);
    _shader.setInt("heightMap", 0);
    _shader.setInt("u_normalMap", 1);

    _vao.bind();
    glDrawArrays(GL_PATCHES, 0, _rez * _rez * 4);
  }
}

void Terrain::reinit() {
  auto start = std::chrono::high_resolution_clock::now();
  generateChunks();
  auto end = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
  // std::cout << "generateChunks Took: " << ms << "ms" << std::endl;

  start = std::chrono::high_resolution_clock::now();
  initTerrain();
  end = std::chrono::high_resolution_clock::now();
  ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
           .count();
  // std::cout << "initTerrain Took: " << ms << "ms" << std::endl;

  generateVertices();
  uploadVertexData();
}

void Frustum::extract(const glm::mat4 &vp) {
  glm::mat4 t = glm::transpose(vp);
  planes[0] = t[3] + t[0]; // left
  planes[1] = t[3] - t[0]; // right
  planes[2] = t[3] + t[1]; // bottom
  planes[3] = t[3] - t[1]; // top
  planes[4] = t[3] + t[2]; // near
  planes[5] = t[3] - t[2]; // far
  for (auto &p : planes)
    p /= glm::length(glm::vec3(p));
}

bool Frustum::intersectsAABB(glm::vec3 min, glm::vec3 max) const {
  for (auto &p : planes) {
    // find the positive vertex (furthest along plane normal)
    glm::vec3 pv(p.x >= 0 ? max.x : min.x, p.y >= 0 ? max.y : min.y,
                 p.z >= 0 ? max.z : min.z);
    // if the positive vertex is outside, the whole box is outside
    if (glm::dot(glm::vec3(p), pv) + p.w < 0.0f)
      return false;
  }
  return true;
}
