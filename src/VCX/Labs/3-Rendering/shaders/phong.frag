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
uniform bool  u_UseBlinn;
uniform float u_Shininess;
uniform bool  u_UseGammaCorrection;
uniform int   u_AttenuationOrder;
uniform float u_BumpMappingBlend;

uniform sampler2D u_DiffuseMap;
uniform sampler2D u_SpecularMap;
uniform sampler2D u_HeightMap;

vec3 Shade(vec3 lightIntensity, vec3 lightDir, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess) {
    // your code here:
    float cosine;
    if(u_UseBlinn)
    {
        vec3 half = normalize(viewDir + lightDir);
        cosine = max(dot(half, normal), 0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        cosine = max(dot(reflectDir, viewDir), 0);
    }
    vec3 specular = specularColor * pow(cosine, shininess);
    vec3 diffuse = diffuseColor * max(dot(lightDir, normal), 0);
    return (diffuse + specular) * lightIntensity;
}

vec3 GetNormal() {
    // Bump mapping from paper: Bump Mapping Unparametrized Surfaces on the GPU
    vec3 vn = normalize(v_Normal);

    // Bump mapping 
    // from paper: Bump Mapping Unparametrized Surfaces on the GPU
    vec3 posDX = dFdx ( v_Position.xyz );  // choose dFdx (#version 420) or dFdxFine (#version 450) here
    vec3 posDY = dFdy ( v_Position.xyz );
    vec3 r1 = cross ( posDY, vn );
    vec3 r2 = cross ( vn , posDX );
    float det = dot (posDX , r1);
    float Hll = texture( u_HeightMap, v_TexCoord ).x;    //-- height from bump map texture, tc=texture coordinates
    float Hlr = texture( u_HeightMap, v_TexCoord + dFdx(v_TexCoord.xy) ).x;
    float Hul = texture( u_HeightMap, v_TexCoord + dFdy(v_TexCoord.xy) ).x;

    // gradient of surface texture. dBs=Hlr-Hll, dBt=Hul-Hll
    vec3 surf_grad = sign(det) * ( (Hlr - Hll) * r1 + (Hul - Hll)* r2 );    
    vec3 vbumpnorm = vn*(1.0 - u_BumpMappingBlend) + u_BumpMappingBlend * normalize ( abs(det)*vn - surf_grad );  // bump normal

    return vbumpnorm;
}

void main() {
    float gamma          = 2.2;
    vec4  diffuseFactor  = texture(u_DiffuseMap , v_TexCoord).rgba;
    vec4  specularFactor = texture(u_SpecularMap, v_TexCoord).rgba;
    if (diffuseFactor.a < .2) discard;
    vec3  diffuseColor   = u_UseGammaCorrection ? pow(diffuseFactor.rgb, vec3(gamma)) : diffuseFactor.rgb;
    vec3  specularColor  = specularFactor.rgb;
    float shininess      = u_Shininess < 0 ? specularFactor.a * 256 : u_Shininess;
    vec3  normal         = GetNormal();
    vec3  viewDir        = normalize(u_ViewPosition - v_Position);
    // Ambient component.
    vec3  total = u_AmbientIntensity * u_AmbientScale * diffuseColor;
    // Iterate lights.
    for (int i = 0; i < u_CntPointLights; i++) {
        vec3  lightDir     = normalize(u_Lights[i].Position - v_Position);
        float dist         = length(u_Lights[i].Position - v_Position);
        float attenuation  = 1. / (u_AttenuationOrder == 2 ? dist * dist : (u_AttenuationOrder == 1  ? dist : 1.));
        total             += Shade(u_Lights[i].Intensity, lightDir, normal, viewDir, diffuseColor, specularColor, shininess) * attenuation;
    }
    for (int i = u_CntPointLights + u_CntSpotLights; i < u_CntPointLights + u_CntSpotLights + u_CntDirectionalLights; i++) {
        total += Shade(u_Lights[i].Intensity, u_Lights[i].Direction, normal, viewDir, diffuseColor, specularColor, shininess);
    }
    // Gamma correction.
    f_Color = vec4(u_UseGammaCorrection ? pow(total, vec3(1. / gamma)) : total, 1.);
}
