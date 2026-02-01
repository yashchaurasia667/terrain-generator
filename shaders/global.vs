#version 330 core

layout (std140) uniform matrices {
  mat4 view;
  mat4 projection;
};
