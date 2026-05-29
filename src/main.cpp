#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <cmath>
#include <iostream>
#include <vector>

#include <camera.h>
#include <indexBuffer.h>
#include <shader.h>
#include <skybox.h>
#include <terrain.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

#define STB_IMAGE_IMPLEMANTATION
#include <texture.h>

// GLOBAL VARIABLES
bool camera_movement = false;
Camera camera(glm::vec3(0.0f), 45.0f, 0.1f, 50.5f);
unsigned int scr_width = 1280, scr_height = 720;

// IMGUI PARAMS
bool wireframe = false, sanity_check = false, render_terrain = true;
float amp = 128.0f, freq = 0.2f, persistance = 0.4f, lacunarity = 2.0f, texScale = 10.0f;
int noisePass = 10;
glm::vec3 lightDir = glm::vec3(0.6f, 1.0f, 0.4f), lightColor = glm::vec3(1.0);

// FUNCTION DECLERATIONS
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void charCallback(GLFWwindow *window, unsigned int codepoint);
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods);

void runNoiseShader(ComputeShader &noiseShader, Terrain &terrain,
                    unsigned int noise_tex, int texResolution) {
  noiseShader.bind();
  noiseShader.setInt("u_seed", terrain.noiseSeed);
  noiseShader.setVec2("u_chunkOffset", glm::vec2(0.0f));
  noiseShader.setInt("u_cellWidth", terrain.cellWidth);
  noiseShader.setInt("u_chunkWidth", terrain.chunkWidth);
  noiseShader.setInt("u_noisePass", noisePass);
  // noiseShader.setFloat("u_amplitude", amp);
  noiseShader.setFloat("u_frequency", freq);
  noiseShader.setFloat("u_lacunarity", lacunarity);
  noiseShader.setFloat("u_persistance", persistance);

  glBindImageTexture(0, noise_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
  noiseShader.setInt("u_heightMap", 0);
  glDispatchCompute((texResolution + 15) / 16, (texResolution + 15) / 16, 1);

  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(scr_width, scr_height, "Terrain Generator", NULL, NULL);
  if (window == nullptr) {
    std::cout << "Failed to create a GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize opengl function pointers" << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCharCallback(window, charCallback);
  glEnable(GL_DEPTH_TEST);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 430 core");

  {
    ComputeShader noiseShader("../shaders/noise_compute.glsl");
    Skybox skybox("../resources/skyboxes/citrus-orchard-road");
    Terrain terrain;
    terrain.initShader("../shaders/chunk_vert.glsl",
                       "../shaders/chunk_frag.glsl", nullptr,
                       "../shaders/tessellation_control.glsl",
                       "../shaders/tessellation_evaluation.glsl");
    int texResolution = terrain.chunkWidth * 4;
    Texture normalMap(
        "../resources/normalMaps/rock_face/rock_face_nor_gl_2k.png");

    unsigned int noise_tex = 0;
    glGenTextures(1, &noise_tex);
    glBindTexture(GL_TEXTURE_2D, noise_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texResolution, texResolution, 0,
                 GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    runNoiseShader(noiseShader, terrain, noise_tex, texResolution);

    // ------------------- SANITY CHECK ------------------------- //
    Shader checkShader("../shaders/shader_vert_default.glsl",
                       "../shaders/shader_frag_default.glsl");
    float check_plane[] = {
        -1.0f, -1.0f, -2.0f, 0.0f, 0.0f, 1.0f,  -1.0f, -2.0f, 1.0f, 0.0f,
        1.0f,  1.0f,  -2.0f, 1.0f, 1.0f, -1.0f, -1.0f, -2.0f, 0.0f, 0.0f,
        1.0f,  1.0f,  -2.0f, 1.0f, 1.0f, -1.0f, 1.0f,  -2.0f, 0.0f, 1.0f};
    VertexArray check_vao;
    VertexBuffer check_vbo(sizeof(check_plane), check_plane, GL_STATIC_DRAW);
    VertexBufferLayout check_layout;
    check_layout.push<float>(3);
    check_layout.push<float>(2);
    check_vao.addBuffer(check_vbo, check_layout);
    // -------------------------------------------------------- //

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glClearColor(0.4, 0.4, 0.4, 0.4);
    while (!glfwWindowShouldClose(window)) {
      glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
      camera.updateFrame();

      glfwPollEvents();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      processInput(window);

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      {
        {
          ImGui::Begin("lighting");
          if (ImGui::CollapsingHeader("light dir")) {
            ImGui::SliderFloat("x", &lightDir.x, -1.0f, 1.0f);
            ImGui::SliderFloat("y", &lightDir.y, -1.0f, 1.0f);
            ImGui::SliderFloat("z", &lightDir.z, -1.0f, 1.0f);
          }
          if (ImGui::CollapsingHeader("light color")) {
            ImGui::SliderFloat("r", &lightColor.x, 0.0f, 1.0f);
            ImGui::SliderFloat("g", &lightColor.y, 0.0f, 1.0f);
            ImGui::SliderFloat("b", &lightColor.z, 0.0f, 1.0f);
          }
          ImGui::End();
        }
        {
          ImGui::Begin("debug");
          ImGui::Checkbox("wireframe", &wireframe);
          ImGui::Checkbox("sanity check", &sanity_check);
          ImGui::Checkbox("render terrain", &render_terrain);
          ImGui::End();
        }
        {
          ImGui::Begin("chunk");
          ImGui::InputInt("chunk width", &terrain.chunkWidth);
          ImGui::InputInt("cell width", &terrain.cellWidth);
          ImGui::InputInt("noise seed", &terrain.noiseSeed);
          ImGui::SliderInt("nosie pass", &noisePass, 1, 64);
          ImGui::SliderFloat("frequency", &freq, 0.0f, 1.0f);
          ImGui::SliderFloat("lacunarity", &lacunarity, 0.0f, 5.0f);
          ImGui::SliderFloat("persistance", &persistance, 0.0f, 1.0f);
          if (ImGui::Button("Reinitialize terrain")) {
            terrain.generateVertices();
            terrain.uploadVertexData();
            runNoiseShader(noiseShader, terrain, noise_tex, texResolution);
          }
          ImGui::End();
        }
        {
          ImGui::Begin("terrain");
          if (ImGui::CollapsingHeader("TESS DISTANCE")) {
            ImGui::SliderFloat("min tessellation distance",
                               &terrain.tess_min_dist, 0.0f, 100.0f);
            ImGui::SliderFloat("max tessellation distance",
                               &terrain.tess_max_dist, 1.0f, 10000.0f);
          }
          if (ImGui::CollapsingHeader("TESS LEVEL")) {
            ImGui::SliderInt("min tessellation level", &terrain.tess_min_level,
                             0.0f, 64.0f);
            ImGui::SliderInt("max tessellation level", &terrain.tess_max_level,
                             4.0f, 128.0f);
          }
          ImGui::SliderFloat("amplitude", &amp, 0.0f, 1000.0f);
          ImGui::SliderFloat("tex scale", &texScale, 0.0f, 100.0f);
          ImGui::End();
        }
      }

      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = camera.getViewMatrix();
      glm::mat4 projection = glm::perspective(
          camera.getFov(), (float)scr_width / (float)scr_height, 0.1f,
          10000.0f);

      if (sanity_check) {
        checkShader.bind();
        checkShader.setMat4("model", model);
        checkShader.setMat4("view", view);
        checkShader.setMat4("projection", projection);
        checkShader.setInt("heightMap", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noise_tex);

        check_vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
      }

      if (render_terrain) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noise_tex);

        terrain.shader.bind();
        terrain.shader.setFloat("u_amplitude", amp);
        terrain.shader.setFloat("u_chunkWidth", terrain.chunkWidth);
        terrain.shader.setFloat("u_texScale", texScale);
        terrain.shader.setInt("u_noisePass", noisePass);
        terrain.shader.setVec3("u_lightDir", glm::normalize(lightDir));
        terrain.shader.setVec3("u_lightColor", glm::normalize(lightColor));
        terrain.shader.setVec3("u_viewPos", camera.getPos());
        terrain.shader.setInt("u_normalMap", 1);

        glActiveTexture(GL_TEXTURE1);
        normalMap.bind();
        terrain.render(camera, model, projection, 0);

        skybox.render(view, projection);
      }

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  scr_width = width;
  scr_height = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  camera.processMovement(window);
}

void cursorPosCallback(GLFWwindow *window, double xposin, double yposin) {
  float xpos = static_cast<float>(xposin);
  float ypos = static_cast<float>(yposin);
  camera.updateView(xpos, ypos);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (ImGui::GetIO().WantCaptureMouse)
    return;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    camera_movement = true;
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    camera.firstMouse = true;
    camera_movement = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, nullptr);
  }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
  float yoff = static_cast<float>(yoffset);
  camera.updateZoom(yoff);
}

// add these function implementations
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void charCallback(GLFWwindow *window, unsigned int codepoint) {
  ImGui_ImplGlfw_CharCallback(window, codepoint);
}
