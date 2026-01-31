#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

class Camera
{
private:
  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;

  float fov;
  float sensitivity, speed;

  float yaw, pitch;
  float lastX, lastY;

  float deltaTime, lastFrame;

public:
  bool firstMouse;
  Camera(glm::vec3 pos, float fov, float sensitivity, float speed);

  glm::mat4 getViewMatrix();
  float getFov();
  glm::vec3 getPos();

  void updateFrame();
  void processMovement(GLFWwindow *window);
  void updateView(float xpos, float ypos);
  void updateZoom(float yoffset);
};
