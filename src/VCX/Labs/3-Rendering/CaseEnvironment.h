#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/UniformBlock.hpp"
#include "Labs/3-Rendering/Content.h"
#include "Labs/3-Rendering/SceneObject.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Labs/Common/ICase.h"

namespace VCX::Labs::Rendering {

    class CaseEnvironment : public Common::ICase {
    public:
        CaseEnvironment(std::initializer_list<Assets::ExampleScene> && scenes);

        virtual std::string_view const GetName() override { return "Environment Mapping"; }
        
        virtual void OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void OnProcessInput(ImVec2 const & pos) override;

    private:
        std::vector<Assets::ExampleScene> const _scenes;

        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueProgram     _skyboxProgram;
        Engine::GL::UniqueRenderFrame _frame;
        SceneObject                   _sceneObject;
        Common::OrbitCameraManager    _cameraManager;
        std::size_t                   _sceneIdx           { 0 };
        bool                          _recompute          { true };
        bool                          _uniformDirty       { true };
        int                           _msaa               { 2 };
        float                         _ambientScale       { 1 };
        float                         _diffuseScale       { 1 };
        float                         _environmentScale   { 1 };
 
        char const *          GetSceneName(std::size_t const i) const { return Content::SceneNames[std::size_t(_scenes[i])].c_str(); }
        Engine::Scene const & GetScene    (std::size_t const i) const { return Content::Scenes[std::size_t(_scenes[i])]; }
    };
}
