#include "Labs/Photon_Mapping/Intersecter.h"

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

} // namespace VCX::Labs::Rendering