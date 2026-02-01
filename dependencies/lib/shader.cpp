#include "shader.h"
#include <utils.h>

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

string Shader::getShaderSource(const char *path)
{
  ifstream shader_file(path);
  if (!shader_file)
  {
    cout << "ERROR::SHADER::" << path << "::NOT SUCCESSFULLY READ" << endl;
    return "";
  }

  cout << "Successfully read shader: " << path << endl; // Add this for debugging
  stringstream buffer;
  buffer << shader_file.rdbuf();
  string source = buffer.str();
  cout << "Shader source length: " << source.length() << endl; // Add this too
  return source;
}

void Shader::checkCompileError(unsigned int id, const char *type)
{
  int success;
  char info[1024];
  if (strcmp(type, "PROGRAM") != 0)
  {
    glCall(glGetShaderiv(id, GL_COMPILE_STATUS, &success));
    if (!success)
    {
      glCall(glGetShaderInfoLog(id, 1024, NULL, info));

      cout << "ERROR::" << type << "::SHADER_COMPILATION_ERROR " << info << endl;
    }
  }
  else
  {
    glCall(glGetProgramiv(id, GL_LINK_STATUS, &success));
    if (!success)
    {
      glCall(glGetProgramInfoLog(id, 1024, NULL, info));

      cout << "ERROR::" << type << "::PROGRAM_LINKING_ERROR " << info << endl;
    }
  }
}

unsigned int Shader::createShader(GLenum type, const char *source, const char *shader_type)
{
  unsigned int id = glCreateShader(type);

  glCall(glShaderSource(id, 1, &source, NULL));
  glCall(glCompileShader(id));
  checkCompileError(id, shader_type);

  return id;
}

Shader::Shader(const char *vertex_path, const char *fragment_path)
{
  std::string vertex_source = getShaderSource(vertex_path);
  std::string fragment_source = getShaderSource(fragment_path);
  unsigned int vertex_shader = createShader(GL_VERTEX_SHADER, vertex_source.c_str(), "VERTEX_SHADER");
  unsigned int fragment_shader = createShader(GL_FRAGMENT_SHADER, fragment_source.c_str(), "FRAGMENT_SHADER");

  ID = glCreateProgram();
  glCall(glAttachShader(ID, vertex_shader));
  glCall(glAttachShader(ID, fragment_shader));
  glCall(glLinkProgram(ID));
  checkCompileError(ID, "PROGRAM");
  glCall(glValidateProgram(ID));

  glCall(glDeleteShader(vertex_shader));
  glCall(glDeleteShader(fragment_shader));
}

Shader::Shader(const char *vertex_path, const char *fragment_path, bool inc_global_vertex, bool inc_global_fragment)
{
  std::string vertex_source = getShaderSource(vertex_path);
  std::string fragment_source = getShaderSource(fragment_path);

  if (inc_global_vertex)
  {
    std::string g_vertex = getShaderSource("../shaders/global.vs");
    vertex_source = vertex_source.substr(vertex_source.find("\n"), vertex_source.length());
    vertex_source = g_vertex + vertex_source;
  }
  if (inc_global_fragment)
  {
    std::string g_fragment = getShaderSource("../shaders/global.fs");
    fragment_source = fragment_source.substr(fragment_source.find("\n"), fragment_source.length());
    fragment_source = g_fragment + fragment_source;
  }

  unsigned int vertex_shader = createShader(GL_VERTEX_SHADER, vertex_source.c_str(), "VERTEX_SHADER");
  unsigned int fragment_shader = createShader(GL_FRAGMENT_SHADER, fragment_source.c_str(), "FRAGMENT_SHADER");

  ID = glCreateProgram();
  glCall(glAttachShader(ID, vertex_shader));
  glCall(glAttachShader(ID, fragment_shader));
  glCall(glLinkProgram(ID));
  checkCompileError(ID, "PROGRAM");
  glCall(glValidateProgram(ID));

  glCall(glDeleteShader(vertex_shader));
  glCall(glDeleteShader(fragment_shader));
}

Shader::Shader(Shader &&other) noexcept : ID(other.ID)
{
  other.ID = 0; // Steal the ID, and set the temporary's ID to 0
}

Shader &Shader::operator=(Shader &&other) noexcept
{
  if (this != &other)
  {
    glDeleteProgram(ID); // Delete the old program if it exists
    ID = other.ID;       // Steal the ID from the temporary object
    other.ID = 0;        // Set temporary's ID to 0 so its destructor does nothing
  }
  return *this;
}

Shader::~Shader()
{
  if (ID != 0)
  {
    glCall(glDeleteProgram(ID));
  }
}

void Shader::addGeometryShader(const char *geometryPath, bool inc_global_geometry)
{
  std::string geometry_source = getShaderSource(geometryPath);
  unsigned int geometry = createShader(GL_GEOMETRY_SHADER, geometry_source.c_str(), "GEOMETRY_SHADER");
  if (inc_global_geometry)
  {
    std::string g_geometry = getShaderSource("../shaders/global.gs");
    geometry_source = geometry_source.substr(geometry_source.find("#version"), geometry_source.length());
    geometry_source = g_geometry + geometry_source;
  }

  glCall(glAttachShader(ID, geometry));
  glCall(glLinkProgram(ID));
  checkCompileError(ID, "PROGRAM");

  glCall(glDeleteShader(geometry));
}

void Shader::bind()
{
  glCall(glUseProgram(ID));
}

void Shader::unbind()
{
  glCall(glUseProgram(0));
}

unsigned int Shader::getId()
{
  return ID;
}

void Shader::setInt(const char *name, int data)
{
  unsigned int loc = glGetUniformLocation(ID, name);
  glCall(glUniform1i(loc, data));
}

void Shader::setFloat(const char *name, float data)
{
  unsigned int loc = glGetUniformLocation(ID, name);
  glCall(glUniform1f(loc, data));
}

void Shader::setBool(const char *name, bool data)
{
  unsigned int loc = glGetUniformLocation(ID, name);
  glCall(glUniform1i(loc, data));
}

void Shader::setVec3(const char *name, glm::vec3 data)
{
  unsigned int loc = glGetUniformLocation(ID, name);
  glCall(glUniform3fv(loc, 1, glm::value_ptr(data)));
}

void Shader::setMat4(const char *name, glm::mat4 data)
{
  unsigned int loc = glGetUniformLocation(ID, name);
  glCall(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(data)));
}
