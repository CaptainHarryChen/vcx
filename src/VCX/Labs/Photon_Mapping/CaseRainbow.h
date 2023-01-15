#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Labs/Photon_Mapping/CaseCaustic.h"
#include "Labs/Photon_Mapping/Content.h"
#include "Labs/Photon_Mapping/Intersecter.h"
#include "Labs/Photon_Mapping/PhotonMapping.h"
#include "Labs/Photon_Mapping/SceneObject.h"

namespace VCX::Labs::Rendering {

    class CaseRainbow : public CaseCaustic {
    protected:

    public:
        CaseRainbow(const std::initializer_list<Assets::ExampleScene> & scenes);
        ~CaseRainbow();

        virtual std::string_view const   GetName() override { return "Rainbow ~~"; }
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
    };
} // namespace VCX::Labs::Rendering
