#version 410 core
layout(location = 0) in vec4 g_Position;
layout(location = 1) in vec2 g_TexCoord;

uniform vec3 u_LightPosition;
uniform float u_FarPlane;

uniform sampler2D u_DiffuseMap;

void main()
{
    vec4 diffuseFactor = texture(u_DiffuseMap , g_TexCoord).rgba;
    if (diffuseFactor.a < .2) discard;
    float lightDistance = length(g_Position.xyz - u_LightPosition);
    lightDistance = lightDistance / u_FarPlane;
    gl_FragDepth = lightDistance;
}