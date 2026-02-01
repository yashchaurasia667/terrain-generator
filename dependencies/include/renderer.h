#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>
#include <string>

#include <shader.h>
#include <camera.h>
#include <model.h>

enum LightTypeList
{
  POINT,
  SPOT
};

struct PointLight
{
  Model model;
  glm::vec3 color;
  float strength;
};

struct DirectionalLight
{
  glm::vec3 direction;
  glm::vec3 color;
  float strength;
};

struct SpotLight
{
  Model model;
  glm::vec3 direction;
  glm::vec3 color;

  float cutoff;
  float oCutoff;
  float strength;
};

struct Skybox
{
  VertexArray vao;
  VertexBuffer vbo;
  unsigned int texId = 0;
  Shader shader;

  Skybox() = default;
};

class Renderer
{
private:
  static float main_scale;
  static GLFWwindow *window;
  static std::vector<Model> models;

  static bool dirLightEnabled;
  static DirectionalLight dirLight;
  static std::vector<PointLight> pointLights;
  static std::vector<SpotLight> spotLights;
  static Skybox *skybox;

  static ImGuiIO *io;
  static Shader lightShader;
  static void drawLights(glm::mat4 view, glm::mat4 projection);

public:
  static int width, height;
  static float ambient, diffuse, specular;
  static GLFWmousebuttonfun glfw_mouse_button_callback;
  static GLFWkeyfun glfw_key_callback;
  static Shader global_shader;

  Renderer(const char *title, int width, int height, const char *glsl_version, bool vsync);
  ~Renderer();
  static void start(void (*game_loop)(GLFWwindow *window, Shader &shader), Shader &shader, Camera &camera);
  static void addModel(std::string path, glm::vec3 position, glm::vec2 rotation, glm::vec3 scale);
  static void addLight(glm::vec3 color, float strength, LightTypeList type, glm::vec3 position, glm::vec3 direction, float cutoff, float outer_cutoff);

  static GLFWwindow *getWindow();
  static Shader &getLightShader();
  static void setSkyBox(std::string path, std::string format);

  static void setMouseButtonCallback(GLFWmousebuttonfun callback);
  static void setKeyCallback(GLFWkeyfun callback);

  static void setCursorMode(unsigned int mode);
  static void setFrameBufferCallback(GLFWframebuffersizefun callback);
  static void setCursorPosCallback(GLFWcursorposfun callback);
  static void setScrollCallback(GLFWscrollfun callback);
  static void setErrorCallback(GLFWerrorfun callback);
};