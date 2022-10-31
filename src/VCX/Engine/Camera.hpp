#pragma once

#include <glm/ext.hpp>

namespace VCX::Engine {
    struct Camera {
    public:
        float     Fovy   = 60.f;
        float     ZNear  = 0.01f;
        float     ZFar   = 100.f;
        glm::vec3 Eye    = glm::vec3(0.f, 0.f, 1.f);
        glm::vec3 Target = glm::vec3(0.f);
        glm::vec3 Up     = glm::vec3(0.f, 1.f, 0.f);

        glm::mat4 GetProjectionMatrix(float aspect) const {
            return glm::perspective(glm::radians(Fovy), aspect, ZNear, ZFar);
        }

        glm::mat4 GetViewMatrix() const {
            return glm::lookAt(Eye, Target, Up);
		}
    };

	class ICameraManager {
	public:
        virtual void Update(Camera & camera) = 0;
	};
}
