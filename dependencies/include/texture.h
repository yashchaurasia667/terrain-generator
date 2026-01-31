#pragma once

class Texture
{
private:
  int width, height, nrChannels;

public:
  unsigned int ID;
  Texture(const char *path);
  ~Texture();

  void bind();
  void unbind();
};