#pragma once

#include <random>
#include <spdlog/spdlog.h>

#include "Engine/Scene.h"
#include "Labs/Photon_Mapping/Intersecter.h"
#include "Labs/Photon_Mapping/Ray.h"

namespace VCX::Labs::Rendering {

    using VCX::Labs::Rendering::Photon;

    struct PhotonMapping {
        Engine::Scene const * InternalScene = nullptr;
        std::vector<Photon>   photons;

        PhotonMapping() = default;

        void InitScene(Engine::Scene const * scene, const RayIntersector & intersector, int nEmittedPhotons = 10000, float P_RR = 0.7f);
    };

} // namespace VCX::Labs::Rendering