#include "Assets/bundled.h"
#include "Labs/3-Rendering/App.h"

namespace VCX::Labs::Rendering {
    using namespace Assets;

    App::App() :
        _ui(Labs::Common::UIOptions { }),
        _caseIllumination({ ExampleScene::Floor, ExampleScene::CornellBox, ExampleScene::WhiteOak, ExampleScene::SportsCar, ExampleScene::BreakfastRoom, ExampleScene::Sibenik, ExampleScene::Sponza, }),
        _caseShadow({ ExampleScene::Teapot, ExampleScene::CornellBox, ExampleScene::SportsCar, ExampleScene::WhiteOak, ExampleScene::Sponza }),
        _caseEnvironment({ ExampleScene::Teapot, ExampleScene::Bunny }),
        _caseNonPhoto({ ExampleScene::Teapot, ExampleScene::Bunny }),
        _caseRayTracing({ ExampleScene::Floor, ExampleScene::CornellBox, ExampleScene::WhiteOak, ExampleScene::SportsCar, ExampleScene::BreakfastRoom, ExampleScene::Sibenik, ExampleScene::Sponza }) {
    }

    void App::OnFrame() {
        _ui.Setup(_cases, _caseId);
    }
}
