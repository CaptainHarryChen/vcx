#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Engine/GL/UniformBlock.hpp"
#include "Labs/3-Rendering/Content.h"
#include "Labs/3-Rendering/SceneObject.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Labs/Common/ICase.h"

namespace VCX::Labs::Rendering {

    class CaseNonPhoto : public Common::ICase {
    public:
        CaseNonPhoto(std::initializer_list<Assets::ExampleScene> && scenes);

        virtual std::string_view const GetName() override { return "Non-Photorealistic Rendering"; }
        
        virtual void OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void OnProcessInput(ImVec2 const & pos) override;

    private:
        std::vector<Assets::ExampleScene> const _scenes;

        Engine::GL::UniqueProgram     _backLineProgram;
        Engine::GL::UniqueProgram     _program;
        Engine::GL::UniqueRenderFrame _frame;
        SceneObject                   _sceneObject;
        Common::OrbitCameraManager    _cameraManager;
        std::size_t                   _sceneIdx           { 0 };
        bool                          _recompute          { true };
        bool                          _uniformDirty       { true };
        int                           _msaa               { 2 };
        float                         _lineWidth          { 3 };
        glm::vec3                     _coolColor          { .2f, .3f, .8f };
        glm::vec3                     _warmColor          { .8f, .3f, .2f };
 
        char const *          GetSceneName(std::size_t const i) const { return Content::SceneNames[std::size_t(_scenes[i])].c_str(); }
        Engine::Scene const & GetScene    (std::size_t const i) const { return Content::Scenes[std::size_t(_scenes[i])]; }
    };
}
