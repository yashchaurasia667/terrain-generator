#include "renderer.h"
#include <iostream>
#include <stdexcept>
#include <stb_image.h>
#include <utils.h>

// ------------------------- STATIC MEMBER DECLERATION ----------------------------------

GLFWwindow *Renderer::window = nullptr;
std::vector<Model> Renderer::models;
int Renderer::width = 0;
int Renderer::height = 0;
float Renderer::ambient = 0.3f, Renderer::diffuse = 1.0f, Renderer::specular = 1.0f;

bool Renderer::dirLightEnabled = true;
DirectionalLight Renderer::dirLight = {
    glm::vec3(0.0f),
    glm::vec3(1.0f),
    1.0f};
std::vector<PointLight> Renderer::pointLights;
std::vector<SpotLight> Renderer::spotLights;

float Renderer::main_scale = 0.0f;
ImGuiIO *Renderer::io = nullptr;
Shader Renderer::lightShader;

Skybox *Renderer::skybox = nullptr;

GLFWmousebuttonfun Renderer::glfw_mouse_button_callback = nullptr;
GLFWkeyfun Renderer::glfw_key_callback = nullptr;
unsigned int loadSkyboxTexture(std::string path, std::string format);

// ------------------------- GLFW CALLBACK FUNCTIONS ------------------------------------

void GLFWMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (Renderer::glfw_mouse_button_callback && !ImGui::GetIO().WantCaptureMouse)
    Renderer::glfw_mouse_button_callback(window, button, action, mods);
}

void GLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

  // Only process game input if ImGui isn't capturing keyboard
  if (ImGui::GetIO().WantCaptureKeyboard)
    return;
  Renderer::glfw_key_callback(window, key, scancode, action, mods);
}

void GLFWCharCallback(GLFWwindow *window, unsigned int c)
{
  ImGui_ImplGlfw_CharCallback(window, c);
}

// ------------------------- RENDERER CONSTRUCTORS/DESTRUCTORS --------------------------

Renderer::Renderer(const char *title, int width, int height, const char *glsl_version, bool vsync = false)
{
  if (window)
    throw std::runtime_error("window is already initialized");

  this->width = width;
  this->height = height;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (window == NULL)
    throw std::runtime_error("Failed to create a GLFW window");
  glfwMakeContextCurrent(window);
  if (vsync)
    glfwSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::runtime_error("Failed to load OpenGL function pointers");

  glCall(glEnable(GL_DEPTH_TEST));

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init(glsl_version);

  glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
  glfwSetKeyCallback(window, GLFWKeyCallback);
  glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);

  lightShader = Shader("../shaders/light.vs", "../shaders/light.fs");

  // INITIALIZE SKYBOX
  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 1.0f,

      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, 1.0f};

  VertexBufferLayout layout;
  layout.push<float>(3);

  skybox = new Skybox();
  skybox->vbo.setData(sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
  skybox->vao.addBuffer(skybox->vbo, layout);
  skybox->texId = loadSkyboxTexture("../resources/default_skybox", "jpg");
  skybox->shader = Shader("../shaders/skybox.vs", "../shaders/skybox.fs");
}

Renderer::~Renderer()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (window)
  {
    glfwDestroyWindow(window);
    window = nullptr;
  }
  glfwTerminate();
}

// ------------------------- MODIFYING FUNCITONS ----------------------------------------

void Renderer::start(void (*game_loop)(GLFWwindow *window, Shader &shader), Shader &shader, Camera &camera)
{
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ADD MODEL WINDOW
    {
      static char path[128] = "../resources/monkey/monkey.obj";
      static float px = 0, py = 0, pz = 0;
      static float rx = 0, ry = 0;
      static float scale = 1.0;

      ImGui::Begin("OpenGL Renderer");
      ImGui::InputText("path", path, 128);

      ImGui::Text("Position");
      ImGui::InputFloat("px", &px);
      ImGui::InputFloat("py", &py);
      ImGui::InputFloat("pz", &pz);

      ImGui::Text("Rotation");
      ImGui::InputFloat("rx", &rx);
      ImGui::InputFloat("ry", &ry);

      ImGui::InputFloat("Scale", &scale);

      if (ImGui::Button("Add Model") && strncmp(path, "\0", 1) != 0)
      {
        std::cout << "Loading model" << std::endl;
        models.push_back(Model(path, glm::vec3(px, py, pz), glm::vec2(rx, ry), glm::vec3(scale), false));
        // models.push_back(Model(path, glm::vec3(0.0f), glm::vec2(1.0f), glm::vec3(1.0f), false));
      }

      ImGui::End();
    }
    // LOADED MODELS WINDOW
    {
      ImGui::Begin("Loaded Stuff");

      if (ImGui::CollapsingHeader("Models"))
      {
        for (unsigned int i = 0; i < models.size(); i++)
        {
          if (ImGui::CollapsingHeader(std::to_string(i).c_str()))
          {

            ImGui::Text("Position");
            ImGui::Text(std::to_string(models[i].position.x).c_str());
            ImGui::Text(std::to_string(models[i].position.y).c_str());
            ImGui::Text(std::to_string(models[i].position.z).c_str());

            ImGui::Text("Rotation");
            ImGui::Text(std::to_string(models[i].rotation.x).c_str());
            ImGui::Text(std::to_string(models[i].rotation.y).c_str());

            ImGui::Text(std::to_string(models[i].scale.x).c_str());
          }
        }
      }

      if (ImGui::CollapsingHeader("Lights"))
      {
        if (ImGui::CollapsingHeader("Point lights"))
        {
          for (unsigned int i = 0; i < pointLights.size(); i++)
          {
            if (ImGui::CollapsingHeader(std::to_string(i).c_str()))
            {
              ImGui::Text("Position");
              ImGui::Text("(%.1f, %.1f, %.1f)", pointLights[i].model.position.x, pointLights[i].model.position.y, pointLights[i].model.position.z);

              ImGui::Text("Color");
              ImGui::Text("(%.1f, %.1f, %.1f)", pointLights[i].color.x, pointLights[i].color.y, pointLights[i].color.z);
            }
          }
        }

        if (ImGui::CollapsingHeader("Spotlights"))
        {
          for (unsigned int i = 0; i < spotLights.size(); i++)
          {
            if (ImGui::CollapsingHeader(std::to_string(i).c_str()))
            {
              ImGui::Text("Position");
              ImGui::Text("(%.1f, %.1f, %.1f)", spotLights[i].model.position.x, spotLights[i].model.position.y, spotLights[i].model.position.z);

              ImGui::Text("Direction");
              ImGui::Text("(%.1f, %.1f, %.1f)", spotLights[i].direction.x, spotLights[i].direction.y, spotLights[i].direction.z);

              ImGui::Text("Color");
              ImGui::Text("(%.1f, %.1f, %.1f)", spotLights[i].color.x, spotLights[i].color.y, spotLights[i].color.z);

              ImGui::Text("Cutoff: %f", spotLights[i].cutoff);
              ImGui::Text("Outer Cutoff: %f", spotLights[i].oCutoff);
            }
          }
        }
      }

      ImGui::End();
    }
    // ADD LIGHTS
    {
      static float px = 0, py = 0, pz = 0;
      static float dx = 0, dy = 0, dz = -1.0f;
      static float r = 1.0f, g = 1.0f, b = 1.0f;
      static float strength = 1.0;
      static float cutoff = 15.0f, oCutoff = 20.0f;
      static int index = 0;
      const char *type[] = {"POINT", "SPOTLIGHT"};

      ImGui::Begin("Add Lights");

      // if type is any other than directional
      ImGui::Text("Position");
      ImGui::InputFloat("px", &px);
      ImGui::InputFloat("py", &py);
      ImGui::InputFloat("pz", &pz);

      // if type is any other than point
      if (index != 0)
      {
        ImGui::Text("Direction");
        ImGui::InputFloat("dx", &dx);
        ImGui::InputFloat("dy", &dy);
        ImGui::InputFloat("dz", &dz);
      }

      ImGui::Text("Color");
      ImGui::InputFloat("r", &r);
      ImGui::InputFloat("g", &g);
      ImGui::InputFloat("b", &b);

      // only aplicable for spotlights
      if (index == 2)
      {
        ImGui::InputFloat("CutOff", &cutoff);
        ImGui::InputFloat("Outer CutOff", &oCutoff);
      }

      ImGui::Combo("Type", &index, type, IM_ARRAYSIZE(type));
      ImGui::SliderFloat("Strength", &strength, 0.0f, 1.0f, "%.2f");

      if (ImGui::Button("Add Light"))
      {
        LightTypeList lightType = POINT;
        if (index == SPOT)
          lightType = SPOT;

        addLight(glm::vec3(r, g, b), strength, lightType, glm::vec3(px, py, pz), glm::vec3(dx, dy, dz), cutoff, oCutoff);
      }

      ImGui::End();
    }
    // CHANGE/TOGGLE DIRECTIONAL LIGHT
    {
      ImGui::Begin("Directional Light");

      ImGui::Text("Direction");
      ImGui::SliderFloat("x", &dirLight.direction.x, -1.0f, 1.0f, "%.2f");
      ImGui::SliderFloat("y", &dirLight.direction.y, -1.0f, 1.0f, "%.2f");
      ImGui::SliderFloat("z", &dirLight.direction.z, -1.0f, 1.0f, "%.2f");

      ImGui::Text("Color");
      ImGui::SliderFloat("r", &dirLight.color.r, 0.0f, 1.0f, "%.2f");
      ImGui::SliderFloat("g", &dirLight.color.g, 0.0f, 1.0f, "%.2f");
      ImGui::SliderFloat("b", &dirLight.color.b, 0.0f, 1.0f, "%.2f");

      ImGui::SliderFloat("Strength", &dirLight.strength, 0.0f, 1.0f, "%.2f");

      ImGui::End();
    }

    if (game_loop && window)
      game_loop(window, shader);

    shader.setInt("numPointLights", pointLights.size());
    shader.setInt("numDirectionalLights", 1);
    shader.setInt("numSpotLights", spotLights.size());

    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(camera.getFov(), (float)Renderer::width / (float)Renderer::height, 0.1f, 100.0f);
    drawLights(view, projection, shader);

    for (unsigned int i = 0; i < models.size(); i++)
      models[i].draw(shader);

    // DRAW SKYBOX
    glCall(glDepthFunc(GL_LEQUAL));
    skybox->shader.bind();
    skybox->shader.setMat4("view", glm::mat4(glm::mat3(view)));
    skybox->shader.setMat4("projection", projection);

    skybox->vao.bind();
    glCall(glActiveTexture(GL_TEXTURE0));
    glCall(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texId));
    skybox->shader.setInt("skybox", 0);
    // std::cout << "Skybox uniform location: " << glGetUniformLocation(skybox->shader.getId(), "skybox") << std::endl;

    glCall(glDrawArrays(GL_TRIANGLES, 0, 36));
    glCall(glDepthFunc(GL_LESS));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }
}

void Renderer::addModel(std::string path, glm::vec3 position, glm::vec2 rotation, glm::vec3 scale)
{
  models.push_back(Model(path, position, rotation, scale, false));
}

void Renderer::addLight(glm::vec3 color, float strength, LightTypeList type, glm::vec3 position, glm::vec3 direction, float cutoff, float outer_cutoff)
{
  std::cout << "Adding light: " << type << std::endl;
  switch (type)
  {
  case POINT:
    pointLights.push_back({Model("../resources/lights/pointLight.obj", position, glm::vec2(0.0f), glm::vec3(1.0f), false), color, strength});
    break;
  case SPOT:
    spotLights.push_back({Model("../resources/lights/spotLight.obj", position, glm::vec2(0.0f), glm::vec3(1.0f), false), direction, color, cutoff, outer_cutoff, strength});
    break;
  };
}

// ------------------------- GETTER FUNCTIONS -------------------------------------------

GLFWwindow *Renderer::getWindow()
{
  return window;
}

Shader &Renderer::getLightShader()
{
  return lightShader;
}

// ------------------------- SETTER FUNCTIONS -------------------------------------------

void Renderer::setSkyBox(std::string path, std::string format)
{
  skybox->texId = loadSkyboxTexture(path, format);
}

void Renderer::setCursorMode(unsigned int mode)
{
  if (window)
    glfwSetInputMode(window, GLFW_CURSOR, mode);
}

void Renderer::setFrameBufferCallback(GLFWframebuffersizefun callback)
{
  if (window)
    glfwSetFramebufferSizeCallback(window, callback);
}

void Renderer::setCursorPosCallback(GLFWcursorposfun callback)
{
  if (window)
    glfwSetCursorPosCallback(window, callback);
}

void Renderer::setMouseButtonCallback(GLFWmousebuttonfun callback)
{
  glfw_mouse_button_callback = callback;
}

void Renderer::setKeyCallback(GLFWkeyfun callback)
{
  glfw_key_callback = callback;
}

void Renderer::setScrollCallback(GLFWscrollfun callback)
{
  if (window)
    glfwSetScrollCallback(window, callback);
}

void Renderer::setErrorCallback(GLFWerrorfun callback)
{
  if (window)
    glfwSetErrorCallback(callback);
}

// ------------------------- HELPER FUNCTIONS ------------------------------------------

void Renderer::drawLights(glm::mat4 view, glm::mat4 projection, Shader &shader)
{
  // DRAW DIRECTIONAL LIGHT
  if (dirLightEnabled)
  {
    shader.bind();
    shader.setVec3("directionalLights[0].direction", dirLight.direction);
    shader.setVec3("directionalLights[0].color", dirLight.color);
    shader.setFloat("directionalLights[0].strength", dirLight.strength);
    shader.setVec3("directionalLights[0].ambient", glm::vec3(ambient));
    shader.setVec3("directionalLights[0].diffuse", glm::vec3(diffuse));
    shader.setVec3("directionalLights[0].specular", glm::vec3(specular));
  }

  // DRAW ALL POINT LIGHTS
  for (unsigned int i = 0; i < pointLights.size(); i++)
  {
    glm::mat4 model = pointLights[i].model.getModelMatrix();
    lightShader.bind();
    lightShader.setMat4("model", model);
    lightShader.setMat4("view", view);
    lightShader.setMat4("projection", projection);
    lightShader.setVec3("color", pointLights[i].color);
    pointLights[i].model.draw(lightShader);

    std::string lightstr = "pointLights";
    lightstr += "[" + std::to_string(i) + "]";
    shader.bind();
    shader.setVec3((lightstr + ".position").c_str(), pointLights[i].model.position);
    shader.setVec3((lightstr + ".color").c_str(), pointLights[i].color);

    shader.setVec3((lightstr + ".ambient").c_str(), glm::vec3(0.05f));
    shader.setVec3((lightstr + ".diffuse").c_str(), glm::vec3(diffuse));
    shader.setVec3((lightstr + ".specular").c_str(), glm::vec3(specular));

    shader.setFloat((lightstr + ".strength").c_str(), pointLights[i].strength);
    shader.setFloat((lightstr + ".constant").c_str(), 1.0f);
    shader.setFloat((lightstr + ".linear").c_str(), 0.22f);
    shader.setFloat((lightstr + ".quadratic").c_str(), 0.20f);
  }

  // DRAW ALL SPOTLIGHTS
  for (unsigned int i = 0; i < spotLights.size(); i++)
  {
    glm::mat4 model = spotLights[i].model.getModelMatrix();

    lightShader.bind();
    lightShader.setMat4("model", model);
    lightShader.setMat4("view", view);
    lightShader.setMat4("projection", projection);
    lightShader.setVec3("color", spotLights[i].color);
    spotLights[i].model.draw(lightShader);

    std::string lightstr = "spotlights";
    lightstr += "[" + std::to_string(i) + "]";
    shader.bind();
    shader.setVec3((lightstr + ".position").c_str(), spotLights[i].model.position);
    shader.setVec3((lightstr + ".direction").c_str(), spotLights[i].direction);
    shader.setVec3((lightstr + ".color").c_str(), spotLights[i].color);

    shader.setVec3((lightstr + ".ambient").c_str(), glm::vec3(0.05f));
    shader.setVec3((lightstr + ".diffuse").c_str(), glm::vec3(diffuse));
    shader.setVec3((lightstr + ".specular").c_str(), glm::vec3(specular));

    shader.setFloat((lightstr + ".strength").c_str(), spotLights[i].strength);
    shader.setFloat((lightstr + ".constant").c_str(), 1.0f);
    shader.setFloat((lightstr + ".linear").c_str(), 0.22f);
    shader.setFloat((lightstr + ".quadratic").c_str(), 0.20f);

    shader.setFloat((lightstr + ".cutOff").c_str(), glm::radians(spotLights[i].cutoff));
    shader.setFloat((lightstr + ".outerCutOff").c_str(), glm::radians(spotLights[i].oCutoff));
  }
}

unsigned int loadSkyboxTexture(std::string path, std::string format)
{
  std::string texFaces[] = {"right", "left", "top", "bottom", "front", "back"};
  unsigned int texId;

  glCall(glGenTextures(1, &texId));
  glCall(glBindTexture(GL_TEXTURE_CUBE_MAP, texId));
  path += '/';

  int width, height, nrChannels;
  for (unsigned int i = 0; i < 6; i++)
  {
    std::string fullPath = path + texFaces[i] + "." + format;
    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
      unsigned int fmt = GL_RED;
      switch (nrChannels)
      {
      case 3:
        fmt = GL_RGB;
        break;
      case 4:
        fmt = GL_RGBA;
        break;
      }

      glCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data));
      stbi_image_free(data);
    }
    else
    {
      std::cout << "Cubemap tex failed to load at path: " << texFaces[i] << std::endl;
      stbi_image_free(data);
    }
  }

  glCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  glCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  glCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  glCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  glCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

  glCall(glBindTexture(GL_TEXTURE_CUBE_MAP, texId));
  return texId;
}
