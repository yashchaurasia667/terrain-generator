#pragma once

class VertexBuffer
{
private:
  unsigned int ID;

public:
  VertexBuffer();
  VertexBuffer(unsigned int size, void *data, unsigned int usage);
  ~VertexBuffer();

  void bind() const;
  void unbind() const;
  void setData(unsigned int size, void *data, unsigned int usage);
};