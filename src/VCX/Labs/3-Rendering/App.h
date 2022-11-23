#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/3-Rendering/CaseEnvironment.h"
#include "Labs/3-Rendering/CaseIllumination.h"
#include "Labs/3-Rendering/CaseNonPhoto.h"
#include "Labs/3-Rendering/CaseShadow.h"
#include "Labs/3-Rendering/CaseRayTracing.h"
#include "Labs/Common/UI.h"

namespace VCX::Labs::Rendering {
    class App : public Engine::IApp {
    private:
        Common::UI         _ui;

        CaseIllumination   _caseIllumination;
        CaseEnvironment    _caseEnvironment;
        CaseNonPhoto       _caseNonPhoto;
        CaseShadow         _caseShadow;
        CaseRayTracing     _caseRayTracing;

        std::size_t        _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = {
            _caseIllumination,
            _caseEnvironment,
            _caseNonPhoto,
            _caseShadow,
            _caseRayTracing,
        };

    public:
        App();

        void OnFrame() override;
    };
}
