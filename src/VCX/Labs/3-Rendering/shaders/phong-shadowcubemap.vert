#version 410 core

layout(location = 0) in  vec3 a_Position;
layout(location = 1) in  vec3 a_Normal;
layout(location = 2) in  vec2 a_TexCoord;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoord;

struct Light {
    vec3  Intensity;
    vec3  Direction;   // For spot and directional lights.
    vec3  Position;    // For point and spot lights.
    float CutOff;      // For spot lights.
    float OuterCutOff; // For spot lights.
};

layout(std140) uniform PassConstants {
    mat4  u_Projection;
    mat4  u_View;
    vec3  u_ViewPosition;
    vec3  u_AmbientIntensity;
    Light u_Lights[4];
    int   u_CntPointLights;
    int   u_CntSpotLights;
    int   u_CntDirectionalLights;
};

void main() {
    v_Position  = a_Position;
    v_Normal    = a_Normal;
    v_TexCoord  = a_TexCoord;
    gl_Position = u_Projection * u_View * vec4(v_Position, 1.);   
}
