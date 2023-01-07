#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/Photon_Mapping/CaseSimple.h"
#include "Labs/Common/UI.h"

namespace VCX::Labs::Rendering {
    class App : public Engine::IApp {
    private:
        Common::UI         _ui;

        CaseSimple     _caseSimple;

        std::size_t        _caseId = 0;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = {
            _caseSimple,
        };

    public:
        App();

        void OnFrame() override;
    };
}
