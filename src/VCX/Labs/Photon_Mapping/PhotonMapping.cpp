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
                    ray = Ray(p.Origin, p.Direction);
                    RayHit rayHit = intersector.IntersectRay(ray);
                    if (! rayHit.IntersectState) // No intersection
                        break;
                    RayReflect rayReflect = DirectionFromBSDF(ray, rayHit);
                    if (rayReflect.Type == ReflectType::None) // Don't need reflection or refraction
                        break;
                    Photon next_p = Photon(rayHit.IntersectPosition, rayReflect.Direction, p.Power * rayReflect.Attenuation);
                    if (rayReflect.Type == ReflectType::Diffuse)
                        photons.push_back(next_p);
                    if(uni01(rand_e) > P_RR) // Russian Roulette: stop
                        break;
                    p = next_p;
                }
            }
        }
    }

} // namespace VCX::Labs::Rendering