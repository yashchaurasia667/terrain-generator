#include "camera.h"

Camera::Camera(glm::vec3 pos, float fov, float sensitivity, float speed)
{
  this->pos = pos;
  this->fov = fov;
  firstMouse = true;
  front = glm::vec3(0.0f, 0.0f, -1.0f);
  up = glm::vec3(0.0f, 1.0f, 0.0f);
  deltaTime = 0.0f;
  lastFrame = 0.0f;
  pitch = 0.0f;
  yaw = -90.0f;
  this->sensitivity = sensitivity;
  this->speed = speed;
}

glm::mat4 Camera::getViewMatrix()
{
  return glm::lookAt(pos, pos + front, up);
}

float Camera::getFov()
{
  return fov;
}

glm::vec3 Camera::getPos()
{
  return pos;
}

void Camera::updateFrame()
{
  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
}

void Camera::processMovement(GLFWwindow *window)
{
  float cameraSpeed = speed * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    pos += front * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    pos -= front * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    pos -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    pos += glm::normalize(glm::cross(front, up)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    pos += up * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    pos -= up * cameraSpeed;
}

void Camera::updateView(float xpos, float ypos)
{
  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch < -89.0f)
    pitch = -89.0f;
  if (pitch > 89.0f)
    pitch = 89.0f;

  glm::vec3 direction = glm::vec3(0.0f);
  direction.x = cos(glm::radians(yaw));
  direction.y = sin(glm::radians(pitch));
  direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
  direction = glm::normalize(direction);

  front = direction;
}

void Camera::updateZoom(float yoffset)
{
  fov -= yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
}
