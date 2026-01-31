#pragma once

#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

class VertexArray
{
private:
  unsigned int ID;

public:
  VertexArray();
  ~VertexArray();

  void bind();
  void unbind();
  void addBuffer(VertexBuffer &vb, VertexBufferLayout &layout);
};