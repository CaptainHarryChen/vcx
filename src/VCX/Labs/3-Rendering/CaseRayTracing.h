#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Labs/3-Rendering/Content.h"
#include "Labs/3-Rendering/SceneObject.h"
#include "Labs/3-Rendering/tasks.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"

namespace VCX::Labs::Rendering {

    class CaseRayTracing : public Common::ICase {
    public:
        CaseRayTracing(std::initializer_list<Assets::ExampleScene> && scenes);
        ~CaseRayTracing();

        virtual std::string_view const GetName() override { return "Whitted-Style Ray Tracing"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

    private:
        std::vector<Assets::ExampleScene> const _scenes;
        Engine::GL::UniqueProgram               _program;
        Engine::GL::UniqueRenderFrame           _frame;
        SceneObject                             _sceneObject;
        Common::OrbitCameraManager              _cameraManager;

        Engine::GL::UniqueTexture2D _texture;
        RayIntersector              _intersector;

        std::size_t                             _sceneIdx { 0 };
        bool                                    _enableZoom { true };
        bool                                    _enableShadow { true };
        int                                     _maximumDepth { 3 };
        int                                     _superSampleRate { 1 };
        std::size_t                             _pixelIndex { 0 };
        bool                                    _stopFlag { true };
        bool                                    _sceneDirty { true };
        bool                                    _treeDirty { true };
        bool                                    _resetDirty { true };
        Common::ImageRGB                        _buffer;
        bool                                    _resizable { true };

        std::thread _task;

        auto GetBufferSize() const { return std::pair(std::uint32_t(_buffer.GetSizeX()), std::uint32_t(_buffer.GetSizeY())); }

        char const *          GetSceneName(std::size_t const i) const { return Content::SceneNames[std::size_t(_scenes[i])].c_str(); }
        Engine::Scene const & GetScene(std::size_t const i) const { return Content::Scenes[std::size_t(_scenes[i])]; }
    };
} // namespace VCX::Labs::Rendering
