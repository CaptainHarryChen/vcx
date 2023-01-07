#include "Assets/bundled.h"
#include "Labs/Photon_Mapping/App.h"

namespace VCX::Labs::Rendering {
    using namespace Assets;

    App::App() :
        _ui(Labs::Common::UIOptions { }),
        _caseSimple({ ExampleScene::Floor, ExampleScene::CornellBox, ExampleScene::WhiteOak, ExampleScene::SportsCar, ExampleScene::BreakfastRoom, ExampleScene::Sibenik, ExampleScene::Sponza }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
