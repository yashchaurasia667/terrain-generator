#include <renderer.h>
#include <shader.h>
#include <camera.h>
#include <utils.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

void gameLoop(GLFWwindow *window, Shader &shader);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void mouseCallback(GLFWwindow *window, double xposin, double yposin);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, int key, int scancodoe, int action, int mods);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void glfwErrorCallback(int error, const char *description);

bool cameraMovement = false;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), 45.0f, 0.1f, 2.5f);

int main()
{
  Renderer ren("renderer window", 1280, 720, "#version 330 core", true);
  Renderer::setFrameBufferCallback(framebufferSizeCallback);
  Renderer::setScrollCallback(scrollCallback);
  Renderer::setErrorCallback(glfwErrorCallback);

  Renderer::setMouseButtonCallback(mouseButtonCallback);
  Renderer::setKeyCallback(processInput);

  Shader def("../shaders/cube.vs", "../shaders/cube.fs");
  Renderer::setSkyBox("../resources/skyboxes/sky3", "png");

  Renderer::start(gameLoop, def, camera);

  return 0;
}

void gameLoop(GLFWwindow *window, Shader &shader)
{
  glCall(glClearColor(0.1f, 0.3f, 0.3f, 1.0f));
  glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

  camera.updateFrame();
  if (cameraMovement)
    camera.processMovement(Renderer::getWindow());

  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 projection = glm::perspective(camera.getFov(), (float)Renderer::width / (float)Renderer::height, 0.1f, 100.0f);

  shader.bind();
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);

  shader.setFloat("material.shininess", 24.0f);
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
  Renderer::width = width;
  Renderer::height = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    Renderer::setCursorMode(GLFW_CURSOR_DISABLED);
    Renderer::setCursorPosCallback(mouseCallback);
    cameraMovement = true;
  }
  else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
  {
    camera.firstMouse = true;
    cameraMovement = false;
    Renderer::setCursorMode(GLFW_CURSOR_NORMAL);
    Renderer::setCursorPosCallback(nullptr);
  }
}

void mouseCallback(GLFWwindow *window, double xposin, double yposin)
{
  // std::cout << "mouse" << std::endl;
  float x = static_cast<float>(xposin);
  float y = static_cast<float>(yposin);
  camera.updateView(x, y);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.updateZoom((float)yoffset);
}

void glfwErrorCallback(int error, const char *description)
{
  std::cout << "ERROR::GLFW::" << description << std::endl;
}