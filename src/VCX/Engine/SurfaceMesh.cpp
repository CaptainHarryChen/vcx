#include <algorithm>

#include <spdlog/spdlog.h>

#include "Engine/SurfaceMesh.h"

namespace VCX::Engine {
    std::vector<glm::vec3> SurfaceMesh::ComputeNormals() const {
        std::vector<glm::vec3> normals(Positions.size(), glm::vec3(0));
        for (std::size_t i = 0; i + 2 < Indices.size(); i += 3) {
            std::uint32_t const * face   = Indices.data() + i;
            glm::vec3 const &     p1     = Positions[face[0]];
            glm::vec3 const &     p2     = Positions[face[1]];
            glm::vec3 const &     p3     = Positions[face[2]];
            glm::vec3             normal = glm::cross(p2 - p1, p3 - p1);
            normals[face[0]] += normal;
            normals[face[1]] += normal;
            normals[face[2]] += normal;
        }
        for (glm::vec3 & normal : normals)
            normal = glm::normalize(normal);
        return normals;
    }

    void SurfaceMesh::NormalizePositions(glm::vec3 const & minAABB, glm::vec3 const & maxAABB) {
        glm::vec3 currMinAABB(std::numeric_limits<float>::max());
        glm::vec3 currMaxAABB(std::numeric_limits<float>::min());
        for (auto const & pos : Positions) {
            currMaxAABB = glm::max(currMaxAABB, pos);
            currMinAABB = glm::min(currMinAABB, pos);
        }
        glm::vec3 currCenter   = (currMaxAABB + currMinAABB) * 0.5f;
        glm::vec3 targetCenter = (maxAABB + minAABB) * 0.5f;
        glm::vec3 currScale    = currMaxAABB - currMinAABB;
        glm::vec3 targetScale  = maxAABB - minAABB;

        glm::vec3 relativeScale(1.0f);
        if (Positions.size() > 1 && targetScale.x > 0 && targetScale.y > 0 && targetScale.z > 0) {
            relativeScale = targetScale / currScale;
        }
        float uniformScale = glm::min(glm::min(relativeScale.x, relativeScale.y), relativeScale.z);

        for (auto & pos : Positions) {
            pos = (pos - currCenter) * uniformScale + targetCenter;
        }
    }
}
