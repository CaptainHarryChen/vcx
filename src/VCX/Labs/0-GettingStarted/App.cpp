#include "Labs/0-GettingStarted/App.h"

namespace VCX::Labs::GettingStarted {

    App::App() :
        _ui(Labs::Common::UIOptions { }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
