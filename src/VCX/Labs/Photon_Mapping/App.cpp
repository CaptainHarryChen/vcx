#include "Labs/Photon_Mapping/App.h"
#include "Assets/bundled.h"

namespace VCX::Labs::Rendering {
    using namespace Assets;

    App::App():
        _ui(Labs::Common::UIOptions {}),
        _caseSimple({ ExampleScene::Floor, ExampleScene::CornellBox, ExampleScene::CornellBoxSphere, ExampleScene::SportsCar, ExampleScene::BreakfastRoom, ExampleScene::Sibenik, ExampleScene::Sponza }) {
        // _caseSimple({ ExampleScene::CornellBoxSphere }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
