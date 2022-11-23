#version 410 core

layout(location = 0) in  vec3 v_Position;
layout(location = 1) in  vec3 v_Normal;
layout(location = 2) in  vec2 v_TexCoord;

layout(location = 0) out vec4 f_Color;

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

uniform float u_AmbientScale;
uniform float u_DiffuseScale;
uniform float u_EnvironmentScale;

uniform sampler2D   u_DiffuseMap;
uniform samplerCube u_EnvironmentMap;

vec3 Shade(vec3 lightIntensity, vec3 lightDir, vec3 normal, vec3 viewDir, vec3 diffuseColor) {
    return max(dot(lightDir, normal), 0.0) * diffuseColor * lightIntensity * u_DiffuseScale;
}

void main() {
    float gamma          = 2.2;
    vec4  diffuseFactor  = texture(u_DiffuseMap , v_TexCoord).rgba;
    vec3  diffuseColor   = pow(diffuseFactor.rgb, vec3(gamma));
    vec3  normal         = normalize(v_Normal);
    vec3  viewDir        = normalize(u_ViewPosition - v_Position);
    // Ambient component.
    vec3  total = u_AmbientIntensity * u_AmbientScale * diffuseColor;
    // Environment component

    // your code here
    total += vec3(0) * u_EnvironmentScale;

    // Iterate lights.
    for (int i = 0; i < u_CntPointLights; i++) {
        vec3  lightDir     = normalize(u_Lights[i].Position - v_Position);
        float dist         = length(u_Lights[i].Position - v_Position);
        float attenuation  = 1. / (dist * dist);
        total             += Shade(u_Lights[i].Intensity, lightDir, normal, viewDir, diffuseColor) * attenuation;
    }
    for (int i = u_CntPointLights + u_CntSpotLights; i < u_CntPointLights + u_CntSpotLights + u_CntDirectionalLights; i++) {
        total += Shade(u_Lights[i].Intensity, u_Lights[i].Direction, normal, viewDir, diffuseColor);
    }
    // Gamma correction.
    f_Color = vec4(pow(total, vec3(1. / gamma)), 1.);
}
