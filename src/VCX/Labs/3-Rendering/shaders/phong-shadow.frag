#version 410 core

layout(location = 0) in  vec3 v_Position;
layout(location = 1) in  vec3 v_Normal;
layout(location = 2) in  vec2 v_TexCoord;
layout(location = 3) in  vec4 v_LightSpacePosition;

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

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SpecularMap;
uniform sampler2D u_ShadowMap;

float Shadow(vec4 lightSpacePosition, vec3 normal, vec3 lightDir) {
    // return 1. if point in shadow, else return 0.
    vec3 pos = lightSpacePosition.xyz / lightSpacePosition.w;
    pos = pos * 0.5 + 0.5;

    // your code here: closestDepth = ?
    float closestDepth = 0;
    // your code end

    float curDepth = pos.z;
    float bias = max(1e-3 * (1.0 - dot(normal, lightDir)), 1e-4);
    float shadow = (curDepth - bias > closestDepth ? 1.0 : 0.0);
    if (pos.z > 1.0 || pos.x < 0. || pos.x > 1. || pos.y < 0. || pos.y > 1.) shadow = 0.0;
    return shadow;
}

vec3 Shade(vec3 lightIntensity, vec3 lightDir, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess) {
    // your code here:
    return vec3(0);
}

void main() {
    float gamma          = 2.2;
    vec4  diffuseFactor  = texture(u_DiffuseMap , v_TexCoord).rgba;
    vec4  specularFactor = texture(u_SpecularMap, v_TexCoord).rgba;
    if (diffuseFactor.a < .2) discard;
    vec3  diffuseColor   = pow(diffuseFactor.rgb, vec3(gamma));
    vec3  specularColor  = specularFactor.rgb;
    float shininess      = specularFactor.a * 256;
    vec3  normal         = normalize(v_Normal);
    vec3  viewDir        = normalize(u_ViewPosition - v_Position);
    // Ambient component.
    vec3  total = u_AmbientIntensity * u_AmbientScale * diffuseColor;
    // Only one light
    float shadow = Shadow(v_LightSpacePosition, normal, u_Lights[0].Direction);
    total += (1. - shadow) * Shade(u_Lights[0].Intensity, u_Lights[0].Direction, normal, viewDir, diffuseColor, specularColor, shininess);
    // Gamma correction.
    f_Color = vec4(pow(total, vec3(1. / gamma)), 1.);
}
