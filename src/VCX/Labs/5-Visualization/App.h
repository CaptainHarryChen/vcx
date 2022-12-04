#pragma once

#include <vector>

#include "Engine/app.h"
#include "Labs/5-Visualization/CaseParallelCoordinates.h"
#include "Labs/5-Visualization/CaseFlowVis.h"
#include "Labs/Common/UI.h"

namespace VCX::Labs::Visualization {
    class App : public Engine::IApp {
    private:
        Common::UI              _ui;
        std::size_t             _caseId = 0;
        CaseParallelCoordinates _caseParallelCoordinates;
        CaseFlowVis             _caseFlowVis;

        std::vector<std::reference_wrapper<Common::ICase>> _cases = {
            _caseParallelCoordinates,
            _caseFlowVis
        };

    public:
        App();

        void OnFrame() override;
    };
}
