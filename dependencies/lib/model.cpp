#include "model.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <mesh.h>
#include <shader.h>
#include <utils.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

// constructor, expects a filepath to a 3D model, position in world space, rotation, scale and gamma correction
Model::Model(std::string const &path, glm::vec3 position, glm::vec2 rotation, glm::vec3 scale, bool gamma = false) : gammaCorrection(gamma)
{
  this->rotation = rotation;
  this->position = position;
  this->scale = scale;
  loadModel(path);
}

glm::mat4 Model::getModelMatrix()
{
  // assuming the rotation values are degrees
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  if (rotation.x != 0.0f)
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  if (rotation.y != 0.0f)
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::scale(model, scale);

  return model;
}

// draws the model, and thus all its meshes
void Model::draw(Shader &shader)
{
  for (unsigned int i = 0; i < meshes.size(); i++)
  {
    shader.bind();
    shader.setMat4("model", getModelMatrix());
    meshes[i].draw(shader);
  }
}

void Model::loadModel(std::string const &path)
{
  // read file via ASSIMP
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
    return;
  }

  directory = path.substr(0, path.find_last_of('/'));

  processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++)
    processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
  // data to fill
  std::vector<VertexType> vertices;
  std::vector<unsigned int> indices;
  std::vector<TextureType> textures;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++)
  {
    VertexType vertex;
    glm::vec3 vector;
    // positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.position = vector;
    // normals
    if (mesh->HasNormals())
    {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.normal = vector;
    }
    // texture coordinates
    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
    {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.texCoords = vec;
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.tangent = vector;
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.bitangent = vector;
    }
    else
      vertex.texCoords = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  std::vector<TextureType> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "material.diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  std::vector<TextureType> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "material.specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<TextureType> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "material.normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<TextureType> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "material.height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  return Mesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<TextureType> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
  // std::cout << "loading material texture" << std::endl;
  // std::cout << mat->GetTextureCount(type) << std::endl;
  std::vector<TextureType> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
  {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++)
    {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
      {
        textures.push_back(textures_loaded[j]);
        skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
        break;
      }
    }
    if (!skip)
    { // if texture hasn't been loaded already, load it
      TextureType texture;
      texture.id = TextureFromFile(str.C_Str(), this->directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      textures_loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
    }
  }
  return textures;
}

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
  // std::cout << "loading texture from file" << std::endl;
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glCall(glGenTextures(1, &textureID));

  int width, height, nrComponents;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glCall(glBindTexture(GL_TEXTURE_2D, textureID));
    glCall(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
    glCall(glGenerateMipmap(GL_TEXTURE_2D));

    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}