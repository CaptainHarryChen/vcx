#include "Labs/5-Visualization/App.h"

namespace VCX::Labs::Visualization {
    App::App() :
        _ui(Labs::Common::UIOptions { }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
