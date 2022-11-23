#version 410 core

layout (location = 0) in vec3 a_Position;
layout (location = 2) in vec2 a_TexCoord;

layout (location = 0) out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.);
}