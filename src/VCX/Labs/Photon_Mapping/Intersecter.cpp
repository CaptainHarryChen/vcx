#include "Labs/Photon_Mapping/Intersecter.h"
#include <random>

namespace VCX::Labs::Rendering {

    glm::vec4 GetTexture(Engine::Texture2D<Engine::Formats::RGBA8> const & texture, glm::vec2 const & uvCoord) {
        if (texture.GetSizeX() == 1 || texture.GetSizeY() == 1) return texture.At(0, 0);
        glm::vec2 uv      = glm::fract(uvCoord);
        uv.x              = uv.x * texture.GetSizeX() - .5f;
        uv.y              = uv.y * texture.GetSizeY() - .5f;
        std::size_t xmin  = std::size_t(glm::floor(uv.x) + texture.GetSizeX()) % texture.GetSizeX();
        std::size_t ymin  = std::size_t(glm::floor(uv.y) + texture.GetSizeY()) % texture.GetSizeY();
        std::size_t xmax  = (xmin + 1) % texture.GetSizeX();
        std::size_t ymax  = (ymin + 1) % texture.GetSizeY();
        float       xfrac = glm::fract(uv.x), yfrac = glm::fract(uv.y);
        return glm::mix(glm::mix(texture.At(xmin, ymin), texture.At(xmin, ymax), yfrac), glm::mix(texture.At(xmax, ymin), texture.At(xmax, ymax), yfrac), xfrac);
    }

    glm::vec4 GetAlbedo(Engine::Material const & material, glm::vec2 const & uvCoord) {
        glm::vec4 albedo       = GetTexture(material.Albedo, uvCoord);
        glm::vec3 diffuseColor = albedo;
        return glm::vec4(glm::pow(diffuseColor, glm::vec3(2.2)), albedo.w);
    }

    bool IntersectTriangle(Intersection & output, Ray const & ray, glm::vec3 const & p1, glm::vec3 const & p2, glm::vec3 const & p3) {
        glm::vec3 normal = glm::cross(p1 - p2, p1 - p3);
        if (abs(glm::dot(ray.Direction, normal)) < EPS2)
            return false;
        output.t = glm::dot(p1 - ray.Origin, normal) / glm::dot(ray.Direction, normal);
        if (output.t <= EPS1)
            return false;
        output.u = glm::dot(p3 - p1, glm::cross(ray.Direction, ray.Origin - p1)) / glm::dot(ray.Direction, normal);
        output.v = glm::dot(-p2 + p1, glm::cross(ray.Direction, ray.Origin - p1)) / glm::dot(ray.Direction, normal);
        if (output.u < 0 || output.v > 1 || output.v < 0 || output.v > 1 || output.u + output.v > 1)
            return false;
        return true;
    }

    glm::vec3 RandomDirection() {
        static std::mt19937                   e;
        std::uniform_real_distribution<float> uni(0, 1);
        float                                 u = uni(e), v = uni(e);
        float                                 theta = 2.0f * std::_Pi * u;
        float                                 phi   = acos(2.0f * v - 1);
        // assert(glm::abs(glm::length(glm::vec3(sin(theta) * sin(phi), cos(theta) * sin(phi), cos(phi))) - 1.0f) < 1e-2f);
        return glm::vec3(sin(theta) * sin(phi), cos(theta) * sin(phi), cos(phi));
    }

    glm::vec3 RandomHemiDirection(const glm::vec3 & normal) {
        glm::vec3 res = RandomDirection();
        float     d   = glm::dot(res, normal);
        if (d < 0.0f)
            res -= normal * d * 2.0f;
        // assert(glm::abs(glm::length(res) - 1.0f) < 1e-2);
        return res;
    }

    glm::vec3 RandomCosineDirection(const glm::vec3 & n, const glm::vec3 &lx) {
        static std::mt19937                   e;
        std::uniform_real_distribution<float> uni(0, 1);
        float phi = uni(e) * 2.0f * glm::pi<float>();
        float theta = uni(e) * glm::asin(uni(e));
        glm::vec3 dir = glm::vec3(cos(phi) * sin(theta), cos(theta), sin(phi) * sin(theta));
        glm::vec3 lz = glm::normalize(glm::cross(n, lx));
        glm::vec3 res = glm::normalize(dir[0] * lx + dir[1] * n + dir[2] * lz);
        return res;
    }

    float schlick(float cosine, float ref_idx) {
        float r0 = (1 - ref_idx) / (1 + ref_idx);
        r0       = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }

    RayReflect DirectionFromBSDF(const Ray & ray, const RayHit & rayHit) {
        assert(rayHit.IntersectState);
        static std::mt19937                   e;
        std::uniform_real_distribution<float> uni01(0, 1);

        float     alpha     = rayHit.IntersectAlbedo.w;
        float     shininess = rayHit.IntersectMetaSpec.w;
        glm::vec3 n         = rayHit.IntersectNormal;
        glm::vec3 ks        = rayHit.IntersectMetaSpec;
        glm::vec3 kd        = rayHit.IntersectAlbedo;
        float     Ior       = rayHit.IntersectIor;
        glm::vec3 tr        = rayHit.IntersectTrans;

        RayReflect res;
        if (rayHit.IntersectMode == Engine::BlendMode::ColorOnly) {
            res.Type        = ReflectType::Set;
            res.Attenuation = kd;
        } else if (rayHit.IntersectMode == Engine::BlendMode::Phong) {
            res.Type            = ReflectType::Diffuse;
            glm::vec3 dir       = RandomHemiDirection(n);
            glm::vec3 h         = glm::normalize(-ray.Direction + dir);
            float     spec_coef = glm::pow(glm::max(glm::dot(h, n), 0.0f), shininess);
            float     diff_coef = glm::max(glm::dot(dir, n), 0.0f);
            res.Attenuation     = (diff_coef * kd + spec_coef * ks) * 2.0f; // probability density is 1/2pi, but the bsdf has a 1/pi
            res.Direction       = dir;
        } else if (rayHit.IntersectMode == Engine::BlendMode::Reflect || rayHit.IntersectMode == Engine::BlendMode::ReflectNoFresnel) {
            res.Type        = ReflectType::Specular;
            res.Direction   = ray.Direction - glm::vec3(2.0f) * n * glm::dot(n, ray.Direction);
            res.Attenuation = ks;
        } else if (rayHit.IntersectMode == Engine::BlendMode::Transparent || rayHit.IntersectMode == Engine::BlendMode::TransparentGlass || rayHit.IntersectMode == Engine::BlendMode::TransparentNoFresnel || rayHit.IntersectMode == Engine::BlendMode::TransparentNoReflect) {
            // Todo: random choose reflect or refract
            float     ior    = 1.0f / Ior;
            glm::vec3 normal = n;
            if (glm::dot(ray.Direction, n) > 0.f) {
                ior    = 1.0f / ior;
                normal = -n;
            }
            float cosi         = glm::dot(-ray.Direction, normal);
            float cost2        = 1.0f - ior * ior * (1.0f - cosi * cosi);
            float reflect_prob = 0.0f;
            if (ior < 1.0f)
                reflect_prob = cost2 <= 0.0f ? 1.0f : schlick(cosi, ior);
            if (uni01(e) <= reflect_prob) {
                res.Type        = ReflectType::Specular;
                res.Direction   = ray.Direction - glm::vec3(2.0f) * normal * glm::dot(n, ray.Direction);
                res.Attenuation = glm::vec3(1.0f);
            } else {
                glm::vec3 t     = ray.Direction * ior + normal * (ior * cosi - sqrt(fabs(cost2)));
                res.Type        = ReflectType::Refraction;
                res.Direction   = t;
                res.Attenuation = tr;
            }
        } else {
            res.Type = ReflectType::None;
        }
        // if(res.Attenuation[0] > 3.0f || res.Attenuation[1] > 3.0f || res.Attenuation[2] > 3.0f)
        //     printf("abnormal attenuation: (%f, %f, %f)\n",res.Attenuation.x, res.Attenuation.y, res.Attenuation.z);
        return res;
    }

    glm::vec3 DirectLight(const RayIntersector & intersector, const Ray & ray, const RayHit & rayHit, bool enableShadow) {
        static std::mt19937                   rand_e;
        std::uniform_real_distribution<float> uni01(0, 1);

        glm::vec3 pos, n, kd, ks;
        float     alpha, shininess;
        pos       = rayHit.IntersectPosition;
        n         = rayHit.IntersectNormal;
        kd        = rayHit.IntersectAlbedo;
        ks        = rayHit.IntersectMetaSpec;
        alpha     = rayHit.IntersectAlbedo.w;
        shininess = rayHit.IntersectMetaSpec.w * 256;

        glm::vec3 color = glm::vec3(0.0f);
        for (const Engine::Light & light : intersector.InternalScene->Lights) {
            glm::vec3 l;
            float     attenuation;
            if (light.Type == Engine::LightType::Point) {
                l           = light.Position - pos;
                attenuation = 1.0f / glm::dot(l, l) / 4.0f / glm::pi<float>();
                if (enableShadow) {
                    auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
                    if (shadowRayHit.IntersectState) {
                        glm::vec3 sh = shadowRayHit.IntersectPosition - pos;
                        if (glm::dot(sh, sh) + 1e-2 < glm::dot(l, l))
                            attenuation = 0.0f;
                    }
                }
            } else if (light.Type == Engine::LightType::Directional) {
                l           = light.Direction;
                attenuation = 1.0f;
                if (enableShadow) {
                    auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
                    if (shadowRayHit.IntersectState)
                        attenuation = 0.0f;
                }
            } else if (light.Type == Engine::LightType::Area) {
                glm::vec3 normal = glm::cross(light.Position2 - light.Position, light.Position3 - light.Position);
                // float area = glm::length(normal);
                normal  = glm::normalize(normal);
                float u = uni01(rand_e), v = uni01(rand_e);
                if (u + v > 1.0f)
                    u = 1 - u, v = 1 - v;
                glm::vec3 lightPos = (1 - u - v) * light.Position + u * light.Position2 + v * light.Position3;
                l                  = lightPos - pos;
                attenuation        = 1.0f / glm::dot(l, l) * glm::max(glm::dot(-l, normal), 0.0f) / glm::pi<float>();
                if (enableShadow) {
                    auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
                    if (shadowRayHit.IntersectState) {
                        glm::vec3 sh = shadowRayHit.IntersectPosition - pos;
                        if (glm::dot(sh, sh) + 1e-2 < glm::dot(l, l))
                            attenuation = 0.0f;
                    }
                }
            }
            glm::vec3 h         = glm::normalize(-ray.Direction + glm::normalize(l));
            float     spec_coef = glm::pow(glm::max(glm::dot(h, n), 0.0f), shininess);
            float     diff_coef = glm::max(glm::dot(glm::normalize(l), n), 0.0f);
            color += light.Intensity * attenuation * (diff_coef * kd + spec_coef * ks);
        }
        return color;
    }

} // namespace VCX::Labs::Rendering