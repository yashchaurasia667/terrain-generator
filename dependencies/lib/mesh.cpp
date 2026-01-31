#include "mesh.h"
#include <utils.h>

Mesh::Mesh(std::vector<VertexType> vertices, std::vector<unsigned int> indices, std::vector<TextureType> textures)
{
  this->vertices = vertices;
  this->indices = indices;
  this->textures = textures;

  setupMesh();
}

void Mesh::draw(Shader &shader)
{
  shader.bind();
  // bind appropriate textures
  unsigned int diffuseNr = 0;
  unsigned int specularNr = 0;
  unsigned int normalNr = 0;
  unsigned int heightNr = 0;
  for (unsigned int i = 0; i < textures.size(); i++)
  {
    glCall(glActiveTexture(GL_TEXTURE0 + i));

    std::string number;
    std::string name = textures[i].type;
    if (name == "material.diffuse")
      number = std::to_string(diffuseNr++);
    else if (name == "material.specular")
      number = std::to_string(specularNr++);
    else if (name == "material.normal")
      number = std::to_string(normalNr++);
    else if (name == "material.height")
      number = std::to_string(heightNr++);

    shader.setInt((name + '[' + number + ']').c_str(), i);
    glCall(glBindTexture(GL_TEXTURE_2D, textures[i].id));
  }

  shader.setInt("material.numDiffuseTextures", diffuseNr + 1);
  shader.setInt("material.numSpecularTextures", specularNr + 1);

  // draw mesh
  glCall(glBindVertexArray(VAO));
  glCall(glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0));
  glCall(glBindVertexArray(0));

  for (unsigned int i = 0; i < textures.size(); i++)
  {
    glCall(glActiveTexture(GL_TEXTURE0 + i));
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
  }
  glCall(glActiveTexture(GL_TEXTURE0));
}

void Mesh::setupMesh()
{
  // create buffers/arrays
  glCall(glGenVertexArrays(1, &VAO));
  glCall(glBindVertexArray(VAO));

  vbo.setData(sizeof(VertexType) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
  ibo.setData(sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

  // vertex Positions
  glCall(glEnableVertexAttribArray(0));
  glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void *)0));
  // vertex normals
  glCall(glEnableVertexAttribArray(1));
  glCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void *)offsetof(VertexType, normal)));
  // vertex texture coords
  glCall(glEnableVertexAttribArray(2));
  glCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void *)offsetof(VertexType, texCoords)));
  // vertex tangent
  glCall(glEnableVertexAttribArray(3));
  glCall(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void *)offsetof(VertexType, tangent)));
  // vertex bitangent
  glCall(glEnableVertexAttribArray(4));
  glCall(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void *)offsetof(VertexType, bitangent)));
  // ids
  glCall(glEnableVertexAttribArray(5));
  glCall(glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexType), (void *)offsetof(VertexType, m_BoneIDs)));
  // weights
  glCall(glEnableVertexAttribArray(6));
  glCall(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void *)offsetof(VertexType, m_Weights)));
  glCall(glBindVertexArray(0));
}