#pragma once

class IndexBuffer
{
private:
  unsigned int ID;

public:
  IndexBuffer();
  IndexBuffer(unsigned int size, unsigned int *data, unsigned int usage);
  ~IndexBuffer();

  void bind() const;
  void unbind() const;
  void setData(unsigned int size, unsigned int *data, unsigned int usage);
};