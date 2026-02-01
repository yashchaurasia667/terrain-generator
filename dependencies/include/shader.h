#pragma once

#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

class Shader
{
private:
  unsigned int ID;
  std::string getShaderSource(const char *path);
  void checkCompileError(unsigned int id, const char *type);
  unsigned int createShader(GLenum type, const char *path, const char *shader_type);

public:
  Shader() = default;
  Shader(const char *vertex_path, const char *fragment_path);
  Shader(const char *vertex_path, const char *fragment_path, bool inc_global_vertex, bool inc_global_fragment);
  Shader(Shader &&other) noexcept;
  Shader &operator=(Shader &&other) noexcept;
  ~Shader();
  void addGeometryShader(const char *geometryPath, bool inc_global_geometry);

  void bind();
  void unbind();
  unsigned int getId();

  void setInt(const char *name, int data);
  void setFloat(const char *name, float data);
  void setBool(const char *name, bool data);
  void setVec3(const char *name, glm::vec3 data);
  void setMat4(const char *name, glm::mat4 data);
};