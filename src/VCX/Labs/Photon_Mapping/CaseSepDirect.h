#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Labs/Photon_Mapping/Content.h"
#include "Labs/Photon_Mapping/Intersecter.h"
#include "Labs/Photon_Mapping/PhotonMapping.h"
#include "Labs/Photon_Mapping/SceneObject.h"
#include "Labs/Photon_Mapping/CaseSimple.h"

namespace VCX::Labs::Rendering {

    class CaseSepDirect : public CaseSimple {
    public:
        CaseSepDirect(const std::initializer_list<Assets::ExampleScene> & scenes);
        ~CaseSepDirect();

        virtual std::string_view const GetName() override { return "Indirect Photon Mapping"; }
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
    };
} // namespace VCX::Labs::Rendering
