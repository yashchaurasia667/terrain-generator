#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <iostream>
#include <vector>

#include <camera.h>
#include <indexBuffer.h>
#include <shader.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

// GLOBAL VARIABLES
bool camera_movement = false;
Camera camera(glm::vec3(0.0f), 45.0f, 0.1f, 50.5f);
unsigned int scr_width = 1280, scr_height = 720;

// IMGUI PARAMS
bool wireframe = true, sanity_check = true, render_terrain = true;
int chunkWidth = 1000, cellWidth = 150, noise_seed = 0;
float tess_min_dist = 2, tess_max_dist = 200;
unsigned int rez = 20;

// FUNCTION DECLERATIONS
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

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
  glEnable(GL_DEPTH_TEST);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 430 core");

  {
    Shader shader("../shaders/chunk_vert.glsl", "../shaders/chunk_frag.glsl",
                  nullptr, "../shaders/tessellation_control.glsl",
                  "../shaders/tessellation_evaluation.glsl");
    ComputeShader noiseShader("../shaders/noise_compute.glsl");

    std::vector<float> vertices;
    for (unsigned int i = 0; i < rez; i++) {
      for (unsigned int j = 0; j < rez; j++) {
        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * i) / (float)rez); // v.x
        vertices.push_back(0.0f);                          // v.y
        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * j) / (float)rez); // v.z
        vertices.push_back(i / (float)rez);                // u
        vertices.push_back(j / (float)rez);                // v

        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * (i + 1)) / (float)rez); // v.x
        vertices.push_back(0.0f);                                // v.y
        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * j) / (float)rez); // v.z
        vertices.push_back((i + 1) / (float)rez);          // u
        vertices.push_back(j / (float)rez);                // v

        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * i) / (float)rez); // v.x
        vertices.push_back(0.0f);                          // v.y
        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * (j + 1)) / (float)rez); // v.z
        vertices.push_back(i / (float)rez);                      // u
        vertices.push_back((j + 1) / (float)rez);                // v

        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * (i + 1)) / (float)rez); // v.x
        vertices.push_back(0.0f);                                // v.y
        vertices.push_back(-chunkWidth / 2.0f +
                           (chunkWidth * (j + 1)) / (float)rez); // v.z
        vertices.push_back((i + 1) / (float)rez);                // u
        vertices.push_back((j + 1) / (float)rez);                // v
      }
    }
    const unsigned int NUM_STRIPS = chunkWidth - 1;
    const unsigned int NUM_VERTS_PER_STRIP = chunkWidth * 2;

    VertexArray vao;
    VertexBuffer vbo(vertices.size() * sizeof(float), &vertices[0],
                     GL_STATIC_DRAW);
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(2);
    vao.addBuffer(vbo, layout);

    unsigned int noise_tex;
    glGenTextures(1, &noise_tex);
    glBindTexture(GL_TEXTURE_2D, noise_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, chunkWidth, chunkWidth, 0,
                 GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ------------------- SANITY CHECK ------------------------- //
    Shader checkShader("../shaders/shader_vert_default.glsl",
                       "../shaders/shader_frag_default.glsl");
    float check_plane[] = {
        -1.0f, -1.0f, -2.0f, 1.0f, -1.0f, -2.0f, 1.0f,  1.0f, -2.0f,
        -1.0f, -1.0f, -2.0f, 1.0f, 1.0f,  -2.0f, -1.0f, 1.0f, -2.0f,
    };
    VertexArray check_vao;
    VertexBuffer check_vbo(sizeof(check_plane), check_plane, GL_STATIC_DRAW);
    VertexBufferLayout check_layout;
    check_layout.push<float>(3);
    check_vao.addBuffer(check_vbo, check_layout);
    // -------------------------------------------------------- //

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
          ImGui::Begin("debug");
          ImGui::Checkbox("wireframe", &wireframe);
          ImGui::Checkbox("sanity check", &sanity_check);
          ImGui::Checkbox("render terrain", &render_terrain);
          ImGui::End();
        }
        {
          ImGui::Begin("chunk");
          ImGui::InputInt("chunk width", &chunkWidth);
          ImGui::InputInt("cell width", &cellWidth);
          ImGui::InputInt("noise seed", &noise_seed);
          ImGui::End();
        }
        {
          ImGui::Begin("Tessellation");
          ImGui::SliderFloat("min tessellation distance", &tess_min_dist, 0.0f,
                             100.0f);
          ImGui::SliderFloat("max tessellation distance", &tess_max_dist, 1.0f,
                             1000.0f);
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

        check_vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
      }

      if (render_terrain) {
        noiseShader.bind();
        noiseShader.setInt("u_seed", noise_seed);
        noiseShader.setVec2("u_chunkOffset", glm::vec2(0.0f));
        noiseShader.setInt("u_cellWidth", cellWidth);
        noiseShader.setInt("u_chunkWidth", chunkWidth);

        glBindImageTexture(0, noise_tex, 0, GL_FALSE, 0, GL_READ_WRITE,
                           GL_RGBA32F);
        noiseShader.setInt("u_heightMap", 0);
        glDispatchCompute((chunkWidth + 15) / 16, (chunkWidth + 15) / 16, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

        shader.bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noise_tex);
        shader.setInt("heightMap", 0);

        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        vao.bind();
        glDrawArrays(GL_PATCHES, 0, rez * rez * 4);
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
  float yoff = static_cast<float>(yoffset);
  camera.updateZoom(yoff);
}
