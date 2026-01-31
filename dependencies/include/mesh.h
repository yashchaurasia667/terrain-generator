#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>
#include <indexBuffer.h>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct VertexType
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::ivec4 m_BoneIDs[MAX_BONE_INFLUENCE];
  glm::vec4 m_Weights[MAX_BONE_INFLUENCE];
};

struct TextureType
{
  unsigned int id;
  std::string type;
  std::string path;
};

class Mesh
{
public:
  std::vector<VertexType> vertices;
  std::vector<unsigned int> indices;
  std::vector<TextureType> textures;
  unsigned int VAO;

  Mesh(std::vector<VertexType> vertices, std::vector<unsigned int> indices, std::vector<TextureType> textures);
  void draw(Shader &shader);

private:
  VertexBuffer vbo;
  IndexBuffer ibo;
  void setupMesh();
};