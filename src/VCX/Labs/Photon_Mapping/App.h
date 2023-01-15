#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/Common/UI.h"
#include "Labs/Photon_Mapping/CaseRainbow.h"
#include "Labs/Photon_Mapping/CaseCaustic.h"
#include "Labs/Photon_Mapping/CaseSepDirect.h"
#include "Labs/Photon_Mapping/CaseSimple.h"

namespace VCX::Labs::Rendering {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseSimple    _caseSimple;
        CaseSepDirect _caseSepDirect;
        CaseCaustic   _caseCaustic;
        CaseRainbow   _caseRainbow;

        std::size_t _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = {
            _caseSimple,
            _caseSepDirect,
            _caseCaustic,
            _caseRainbow,
        };

    public:
        App();

        void OnFrame() override;
    };
}
