#include "texture.h"
#include <utils.h>
#include <iostream>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const char *path)
{
  width = height = nrChannels = 0;
  ID = 0;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
  ASSERT(data);
  glCall(glGenTextures(1, &ID));
  glCall(glBindTexture(GL_TEXTURE_2D, ID));

  glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
  glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

  glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
  glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

  unsigned int format = GL_RGB;
  if (nrChannels == 1)
    format = GL_RED;
  else if (nrChannels == 3)
    format = GL_RGB;
  else if (nrChannels == 4)
    format = GL_RGBA;

  glCall(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
  glCall(glGenerateMipmap(GL_TEXTURE_2D));

  stbi_image_free(data);
}

Texture::~Texture()
{
  glCall(glDeleteTextures(1, &ID));
}

void Texture::bind()
{
  glCall(glBindTexture(GL_TEXTURE_2D, ID));
}

void Texture::unbind()
{
  glCall(glBindTexture(GL_TEXTURE_2D, 0));
}
