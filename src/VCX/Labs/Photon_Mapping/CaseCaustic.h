#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Labs/Photon_Mapping/CaseSimple.h"
#include "Labs/Photon_Mapping/Content.h"
#include "Labs/Photon_Mapping/Intersecter.h"
#include "Labs/Photon_Mapping/PhotonMapping.h"
#include "Labs/Photon_Mapping/SceneObject.h"

namespace VCX::Labs::Rendering {

    class CaseCaustic : public CaseSimple {
    protected:
        PhotonMapping & _globalPhotonMapping = _photonmapping;
        PhotonMapping   _causticPhotonMapping;

        Engine::GL::UniqueRenderItem & _globalPointItem = _pointItem;
        Engine::GL::UniqueRenderItem   _causticPointItem;

        std::vector<glm::vec3> & globalPhoton_pos = photon_pos;
        std::vector<glm::vec3>   causticPhoton_pos;

    public:
        CaseCaustic(const std::initializer_list<Assets::ExampleScene> & scenes);
        ~CaseCaustic();

        virtual std::string_view const   GetName() override { return "Caustic Photon Mapping"; }
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
    };
} // namespace VCX::Labs::Rendering
