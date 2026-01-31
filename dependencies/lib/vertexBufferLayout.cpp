#include "vertexBufferLayout.h"

unsigned int VertexBufferLayout::getStride() const
{
  return stride;
}

template <typename T>
void VertexBufferLayout::push(unsigned int count)
{
  static_assert(false);
}

template <>
void VertexBufferLayout::push<float>(unsigned int count)
{
  struct Attribute v = {count, GL_FLOAT, GL_FALSE};
  layout.push_back(v);
  stride += count * sizeof(float);
}

template <>
void VertexBufferLayout::push<unsigned int>(unsigned int count)
{
  struct Attribute v = {count, GL_UNSIGNED_INT, GL_FALSE};
  layout.push_back(v);
  stride += count * sizeof(unsigned int);
}

template <>
void VertexBufferLayout::push<unsigned char>(unsigned int count)
{
  struct Attribute v = {count, GL_UNSIGNED_BYTE, GL_TRUE};
  layout.push_back(v);
  stride += count * sizeof(unsigned char);
}