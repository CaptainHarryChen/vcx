#include "Labs/Photon_Mapping/PhotonMapping.h"

namespace VCX::Labs::Rendering {

    void PhotonMapping::InitScene(Engine::Scene const * scene, const RayIntersector & intersector, int nEmittedPhotons, float P_RR) {
        static std::mt19937                   rand_e;
        std::uniform_real_distribution<float> uni01(0, 1);
        InternalScene = scene;
        photons.clear();
        for (const Engine::Light & light : InternalScene->Lights) {
            for (int i = 0; i < nEmittedPhotons; i++) {
                Photon p;
                Ray    ray;
                if (light.Type == Engine::LightType::Point) {
                    p.Origin    = light.Position;
                    p.Direction = RandomDirection();
                    p.Power     = light.Intensity;
                } else if (light.Type == Engine::LightType::Directional) {
                    // Todo
                }
                while (true) {
                    ray           = Ray(p.Origin, p.Direction);
                    RayHit rayHit = intersector.IntersectRay(ray);
                    if (! rayHit.IntersectState) // No intersection
                        break;
                    RayReflect rayReflect = DirectionFromBSDF(ray, rayHit);
                    if (rayReflect.Type == ReflectType::None) // Don't need reflection or refraction
                        break;
                    if (rayReflect.Type == ReflectType::Diffuse)
                        photons.emplace_back(rayHit.IntersectPosition, p.Direction, p.Power);
                    if (uni01(rand_e) > P_RR) // Russian Roulette: stop
                        break;
                    Photon next_p = Photon(rayHit.IntersectPosition, p.Direction, p.Power * rayReflect.Attenuation);
                    p             = next_p;
                }
            }
        }
        tree.Build(photons);
    }

    glm::vec3 PhotonMapping::CollatePhotons(const RayHit & rayHit, const glm::vec3 & out_dir, int numPhotons) const {
        std::vector<Photon> near_photons;
        tree.NearestKPhotons(rayHit.IntersectPosition, numPhotons, near_photons);

        float     shininess = rayHit.IntersectMetaSpec.w;
        glm::vec3 n         = rayHit.IntersectNormal;
        glm::vec3 ks        = rayHit.IntersectMetaSpec;
        glm::vec3 kd        = rayHit.IntersectAlbedo;

        float     radius2 = 0.0f;
        glm::vec3 pos     = rayHit.IntersectPosition;
        glm::vec3 flux    = glm::vec3(0.0f);
        for (auto p : near_photons) {
            radius2             = glm::max(radius2, glm::dot(p.Origin - pos, p.Origin - pos));
            glm::vec3 h         = glm::normalize(-p.Direction + out_dir);
            float     spec_coef = glm::pow(glm::max(glm::dot(h, n), 0.0f), shininess);
            float     diff_coef = glm::max(glm::dot(out_dir, n), 0.0f);
            flux += p.Power * (diff_coef * kd + spec_coef * ks);
        }
        flux /= (glm::pi<float>() * radius2 * photons.size());
        return flux;
    }

} // namespace VCX::Labs::Rendering