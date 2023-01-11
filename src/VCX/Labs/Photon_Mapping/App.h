#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/Common/UI.h"
#include "Labs/Photon_Mapping/CaseSepDirect.h"
#include "Labs/Photon_Mapping/CaseSimple.h"

namespace VCX::Labs::Rendering {
    class App : public Engine::IApp {
    private:
        Common::UI _ui;

        CaseSimple    _caseSimple;
        CaseSepDirect _caseSepDirect;

        std::size_t _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = {
            _caseSimple,
            _caseSepDirect
        };

    public:
        App();

        void OnFrame() override;
    };
}
