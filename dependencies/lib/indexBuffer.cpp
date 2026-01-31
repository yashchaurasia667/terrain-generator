#include "indexBuffer.h"
#include <utils.h>

#include <glad/glad.h>

IndexBuffer::IndexBuffer() : ID(0)
{
  glCall(glGenBuffers(1, &ID));
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID));
}

IndexBuffer::IndexBuffer(unsigned int size, unsigned int *data, unsigned int usage) : IndexBuffer()
{
  glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage));
}

IndexBuffer::~IndexBuffer()
{
  glCall(glDeleteBuffers(1, &ID));
}

void IndexBuffer::bind() const
{
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID));
}

void IndexBuffer::unbind() const
{
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::setData(unsigned int size, unsigned int *data, unsigned int usage)
{
  glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID));
  glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage));
}
