#version 410 core

layout(location = 0) in  vec3 a_Position;
layout(location = 1) in  vec3 a_Normal;

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

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;
uniform float u_LineWidth;

void main() {
    vec4 clipPos = u_Projection * u_View * vec4(a_Position, 1.);
    vec3 clipNorm = mat3(u_Projection) * mat3(u_View) * a_Normal;
    vec2 offset = normalize(clipNorm.xy) / vec2(u_ScreenWidth, u_ScreenHeight) * u_LineWidth * clipPos.w * 2;
    clipPos.xy += offset;
    gl_Position = clipPos;
}
