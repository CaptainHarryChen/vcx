#pragma once

#include "Assets/bundled.h"
#include "Engine/Scene.h"

namespace VCX::Labs::Rendering {
    class Content {
    public:
        static std::array<Engine::Scene, Assets::ExampleScenes.size()> const Scenes;
        static std::array<std::string,   Assets::ExampleScenes.size()> const SceneNames;
    };
}
