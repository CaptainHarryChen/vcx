#pragma once

#include <glm/glm.hpp>

namespace VCX::Labs::Rendering {

    struct Ray {
        glm::vec3 Origin { 0, 0, 0 };
        glm::vec3 Direction { 0, 0, 0 };
        Ray() = default;
        Ray(const glm::vec3 & orig, const glm::vec3 & dir):
            Origin { orig }, Direction { dir } {}
    };
} // namespace VCX::Labs::Rendering
