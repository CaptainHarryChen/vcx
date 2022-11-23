#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

uniform mat4 u_lightSpaceMatrix;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = u_lightSpaceMatrix * vec4(a_Position, 1.);
}