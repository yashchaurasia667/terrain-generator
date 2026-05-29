#pragma once

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <shader.h>
#include <utils.h>

#include <iostream>
#include <string.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Skybox {
public:
  Skybox(const char *skybox);
  ~Skybox();

  void render(glm::mat4 view, glm::mat4 projection);

private:
  unsigned int ID;
  unsigned int cubemap;
};

Skybox::Skybox(const char *skybox) {
  unsigned int vertex_shader = 0;
  unsigned int fragment_shader = 0;
  std::vector<std::string> faces = {"px.png", "nx.png", "py.png",
                                    "ny.png", "pz.png", "nz.png"};
  glGenTextures(1, &cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  stbi_set_flip_vertically_on_load(false);
  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    std::string path = std::string(skybox) + "/" + faces[i];
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height,
                   0, format, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cerr << "ERROR::CUBEMAP: Failed to load face: " << path << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  ID = glCreateProgram();

  std::string vertex_source = "#version 330 core\n\
    layout(location = 0) in vec3 aPos;\n\
    out vec3 TexCoords;\n\
    uniform mat4 view;\n\
    uniform mat4 projection;\n\
    void main() {\n\
      TexCoords = aPos;\n\
      vec4 pos = projection * view * vec4(aPos, 1.0);\n\
      gl_Position = pos.xyww;\n\
    }";
  std::string fragment_source = "#version 330 core\n\
  out vec4 FragColor;\n\
  in vec3 TexCoords;\n\
  uniform samplerCube u_skybox;\n\
  void main()\n\
  {\n\
    FragColor = texture(u_skybox, TexCoords);\n\
  }";

  vertex_shader =
      createShader(GL_VERTEX_SHADER, vertex_source, "VERTEX_SHADER");
  glCall(glAttachShader(ID, vertex_shader));
  fragment_shader =
      createShader(GL_FRAGMENT_SHADER, fragment_source, "FRAGMENT_SHADER");
  glCall(glAttachShader(ID, fragment_shader));

  glCall(glLinkProgram(ID));
  checkCompileError(ID, "PROGRAM");
  glCall(glValidateProgram(ID));
  if (vertex_shader != 0)
    glCall(glDeleteShader(vertex_shader));
  if (fragment_shader != 0)
    glCall(glDeleteShader(fragment_shader));
}

void Skybox::render(glm::mat4 view, glm::mat4 projection) {
  glDepthFunc(GL_LEQUAL);
  glCall(glUseProgram(ID));

  glm::mat4 skyView = glm::mat4(glm::mat3(view));

  unsigned int loc = glGetUniformLocation(ID, "view");
  glCall(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(skyView)));

  loc = glGetUniformLocation(ID, "projection");
  glCall(glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection)));

  loc = glGetUniformLocation(ID, "u_skybox");
  glCall(glUniform1i(loc, 0));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
  renderCube();
  glDepthFunc(GL_LESS);
}

Skybox::~Skybox() { glDeleteProgram(ID); }
