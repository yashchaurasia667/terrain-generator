#ifndef UTILS_H
#define UTILS_H

#include <glad/glad.h>
#include <iostream>

#define ASSERT(x) \
  if (!x)         \
  __debugbreak()

#define glCall(x) \
  glClearError(); \
  x;              \
  ASSERT(glLogError(#x, __FILE__, __LINE__))


void glClearError();
bool glLogError(const char *function, const char *file, int line);

#endif