#include "Labs/Photon_Mapping/App.h"
#include "Assets/bundled.h"

namespace VCX::Labs::Rendering {
    using namespace Assets;

    const static std::initializer_list<ExampleScene> & scenes = {
        ExampleScene::Floor,
        ExampleScene::CornellBox,
        ExampleScene::CornellBoxSphere,
        ExampleScene::CornellBoxMirror,
        ExampleScene::CornellBoxWater,
        ExampleScene::TriangularPrism
    };

    App::App():
        _ui(Labs::Common::UIOptions {}),
        _caseSimple(scenes),
        _caseSepDirect(scenes),
        _caseCaustic(scenes),
        _caseRainbow({ExampleScene::TriangularPrism}) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
