#include "Labs/Photon_Mapping/PhotonMapping.h"

namespace VCX::Labs::Rendering {

    Photon PhotonMapping::GeneratePhoton(Engine::Light light, int totalNum) {
        static std::mt19937                   rand_e;
        std::uniform_real_distribution<float> uni01(0, 1);
        Photon                                p;
        Ray                                   ray;
        // float magic = 2.0f;
        if (light.Type == Engine::LightType::Point) {
            p.Origin    = light.Position;
            p.Direction = RandomDirection();
            p.Power     = light.Intensity / glm::vec3(totalNum);
        } else if (light.Type == Engine::LightType::Directional) {
            // Todo
        } else if (light.Type == Engine::LightType::Area) {
            glm::vec3 normal = glm::cross(light.Position2 - light.Position, light.Position3 - light.Position);
            float area = glm::length(normal);
            normal           = glm::normalize(normal);
            float u = uni01(rand_e), v = uni01(rand_e);
            if (u + v > 1.0f)
                u = 1 - u, v = 1 - v;
            p.Origin    = (1 - u - v) * light.Position + u * light.Position2 + v * light.Position3;
            p.Direction = RandomCosineDirection(normal, glm::normalize(light.Position2 - light.Position));
            p.Power     = light.Intensity / glm::vec3(totalNum);
        }
        // p.Power *= magic;
        return p;
    }

    void PhotonMapping::InitScene(Engine::Scene const * scene, const RayIntersector & intersector, bool useDirect, int nEmittedPhotons, float P_RR) {
        static std::mt19937                   rand_e;
        std::uniform_real_distribution<float> uni01(0, 1);
        InternalScene = scene;
        photons.clear();
        float perPhoton = 0.99f / InternalScene->Lights.size() / nEmittedPhotons;
        if(progress)
            *progress = 0.0f;
        for (const Engine::Light & light : InternalScene->Lights) {
            for (int i = 0; i < nEmittedPhotons; i++) {
                Photon p = GeneratePhoton(light, nEmittedPhotons);
                Ray    ray;
                bool   isIndirect = false;
                while (true) {
                    ray           = Ray(p.Origin, p.Direction);
                    RayHit rayHit = intersector.IntersectRay(ray);
                    if (! rayHit.IntersectState) // No intersection
                        break;
                    RayReflect rayReflect = DirectionFromBSDF(ray, rayHit);
                    if (rayReflect.Type == ReflectType::None) // Don't need reflection or refraction
                        break;
                    if (rayReflect.Type == ReflectType::Diffuse) {
                        if (useDirect || isIndirect)
                            photons.emplace_back(rayHit.IntersectPosition, p.Direction, p.Power);
                        isIndirect = true;
                    }
                    if (uni01(rand_e) > P_RR) // Russian Roulette: stop
                        break;
                    Photon next_p = Photon(rayHit.IntersectPosition, rayReflect.Direction, p.Power * rayReflect.Attenuation);
                    p             = next_p;
                }
                *progress += perPhoton;
                if(onInit && !*onInit)
                    return;
            }
        }
        tree.Build(photons);
        *progress = 1.0f;
        // debug
        // glm::vec3 sum(0.0f);
        // for(auto &p : photons) {
        //     float thr = 350.0f;
        //     sum += p.Power;
        //     if(p.Power[0] > thr || p.Power[1] > thr || p.Power[2] > thr) {
        //         printf("(%f, %f, %f)\n",p.Power[0],p.Power[1],p.Power[2]);
        //     }
        // }
        // sum = sum / (1.0f * photons.size());
        // printf("avg: (%f, %f, %f)\n",sum[0],sum[1],sum[2]);
        // glm::vec3 li = InternalScene->Lights[0].Intensity;
        // printf("light: (%f, %f, %f)\n",li[0],li[1],li[2]);
        // printf("num: %d\n", nEmittedPhotons);
        // li = li / (1.0f * nEmittedPhotons);
        // printf("initial: (%f, %f, %f)\n",li[0],li[1],li[2]);
    }

    void PhotonMapping::InitCaustic(Engine::Scene const * scene, const RayIntersector & intersector, int nEmittedPhotons, float P_RR) {
        static std::mt19937                   rand_e;
        std::uniform_real_distribution<float> uni01(0, 1);
        InternalScene = scene;
        photons.clear();
        float perPhoton = 0.99f / InternalScene->Lights.size() / nEmittedPhotons;
        if(progress)
            *progress = 0.0f;
        for (const Engine::Light & light : InternalScene->Lights) {
            for (int i = 0; i < nEmittedPhotons; i++) {
                Photon p = GeneratePhoton(light, nEmittedPhotons);
                Ray    ray;
                bool isCaustic = false;
                while (true) {
                    ray           = Ray(p.Origin, p.Direction);
                    RayHit rayHit = intersector.IntersectRay(ray);
                    if (! rayHit.IntersectState) // No intersection
                        break;
                    RayReflect rayReflect = DirectionFromBSDF(ray, rayHit);
                    if (rayReflect.Type == ReflectType::None) // Don't need reflection or refraction
                        break;
                    if (rayReflect.Type == ReflectType::Specular || rayReflect.Type == ReflectType::Refraction)
                        isCaustic = true;
                    if (rayReflect.Type == ReflectType::Diffuse) {
                        if (isCaustic)
                            photons.emplace_back(rayHit.IntersectPosition, p.Direction, p.Power);
                        break;
                    }
                    if (uni01(rand_e) > P_RR) // Russian Roulette: stop
                        break;
                    Photon next_p = Photon(rayHit.IntersectPosition, rayReflect.Direction, p.Power * rayReflect.Attenuation);
                    p             = next_p;
                }
                *progress += perPhoton;
                if(onInit && !*onInit)
                    return;
            }
        }
        tree.Build(photons);
        *progress = 1.0f;
    }

    void PhotonMapping::InitCausticDispersion(Engine::Scene const * scene, const RayIntersector & intersector, int nEmittedPhotons, float P_RR) {
        static std::mt19937                   rand_e;
        std::uniform_real_distribution<float> uni01(0, 1);
        InternalScene = scene;
        photons.clear();
        float perPhoton = 0.99f / InternalScene->Lights.size() / nEmittedPhotons;
        if(progress)
            *progress = 0.0f;
        float magic = 5.0f;
        for (const Engine::Light & light : InternalScene->Lights) {
            for (int i = 0; i < nEmittedPhotons; i++) {
                Photon p = GeneratePhoton(light, nEmittedPhotons);
                for(int k = 0; k < 3; k++)
                    if(i % 3 != k)
                        p.Power[k] = 0.0f;
                    else
                        p.Power[k] *= 3.0f;
                p.Power *= magic;
                Ray    ray;
                bool isCaustic = false;
                while (true) {
                    ray           = Ray(p.Origin, p.Direction);
                    RayHit rayHit = intersector.IntersectRay(ray);
                    if (! rayHit.IntersectState) // No intersection
                        break;
                    RayReflect rayReflect = DirectionFromBSDF_Dispersion(ray, rayHit, i % 3);
                    if (rayReflect.Type == ReflectType::None) // Don't need reflection or refraction
                        break;
                    if (rayReflect.Type == ReflectType::Specular || rayReflect.Type == ReflectType::Refraction)
                        isCaustic = true;
                    if (rayReflect.Type == ReflectType::Diffuse) {
                        if (isCaustic)
                            photons.emplace_back(rayHit.IntersectPosition, p.Direction, p.Power);
                        break;
                    }
                    if (uni01(rand_e) > P_RR) // Russian Roulette: stop
                        break;
                    Photon next_p = Photon(rayHit.IntersectPosition, rayReflect.Direction, p.Power * rayReflect.Attenuation);
                    p             = next_p;
                }
                *progress += perPhoton;
                if(onInit && !*onInit)
                    return;
            }
        }
        tree.Build(photons);
        *progress = 1.0f;
    }

    glm::vec3 PhotonMapping::CollatePhotons(const RayHit & rayHit, const glm::vec3 & out_dir, int numPhotons, float mx_dis) const {
        if (photons.size() == 0)
            return glm::vec3(0.0f);
        std::vector<Photon> near_photons;
        tree.NearestKPhotons(rayHit.IntersectPosition, numPhotons, near_photons, mx_dis);
        if (near_photons.size() == 0)
            return glm::vec3(0.0f);

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
        flux /= (glm::pi<float>() * radius2);
        return flux;
    }

} // namespace VCX::Labs::Rendering