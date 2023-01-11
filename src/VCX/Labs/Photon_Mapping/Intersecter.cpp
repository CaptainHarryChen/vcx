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
        return glm::vec3(sin(theta) * sin(phi), cos(theta) * sin(phi), cos(phi));
    }

    glm::vec3 RandomHemiDirection(const glm::vec3 & normal) {
        glm::vec3 res = RandomDirection();
        float     d   = glm::dot(res, normal);
        if (d < 0.0f)
            res -= normal * d * 2.0f;
        return res;
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
        if (rayHit.IntersectMode == Engine::BlendMode::Phong) {
            res.Type            = ReflectType::Diffuse;
            glm::vec3 dir       = RandomHemiDirection(n);
            glm::vec3 h         = glm::normalize(-ray.Direction + dir);
            float     spec_coef = glm::pow(glm::max(glm::dot(h, n), 0.0f), shininess);
            float     diff_coef = glm::max(glm::dot(dir, n), 0.0f);
            res.Attenuation     = (diff_coef * kd + spec_coef * ks) * 2.0f * glm::pi<float>();
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
            float     cosi  = glm::dot(-ray.Direction, normal);
            float     cost2 = 1.0f - ior * ior * (1.0f - cosi * cosi);
            glm::vec3 t     = ray.Direction * ior + normal * (ior * cosi - sqrt(fabs(cost2)));
            res.Type        = ReflectType::Refraction;
            res.Direction   = cost2 > 0 ? t : glm::vec3(0.0f);
            res.Attenuation = tr;
        } else {
            res.Type = ReflectType::None;
        }
        return res;
    }

} // namespace VCX::Labs::Rendering