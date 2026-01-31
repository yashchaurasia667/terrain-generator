#pragma once

#include <vector>
#include <string>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <mesh.h>

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

class Model
{
private:
  std::vector<Mesh> meshes;
  std::vector<TextureType> textures_loaded;
  std::string directory;
  bool gammaCorrection;

  void loadModel(std::string const &path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<TextureType> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

public:
  glm::vec2 rotation;
  glm::vec3 position;
  glm::vec3 scale;

  Model(std::string const &path, glm::vec3 position, glm::vec2 rotation, glm::vec3 scale, bool gamma);
  glm::mat4 getModelMatrix();
  void draw(Shader &shader);
};