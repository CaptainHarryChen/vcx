#version 410 core

layout (location = 0) in vec2 v_TexCoord;

uniform sampler2D u_DiffuseMap;

void main() {
    vec4 diffuseFactor = texture(u_DiffuseMap , v_TexCoord).rgba;
    if (diffuseFactor.a < .2) discard;
}