#include "utils.h"

void glClearError()
{
  while (glGetError() != GL_NO_ERROR)
    ;
}

bool glLogError(const char *function, const char *file, int line)
{
  while (GLenum error = glGetError())
  {
    std::cout << "[OpenGL Error]: (" << error << "):" << function << " " << file << ":" << line << std::endl;
    return false;
  }
  return true;
}
