#pragma once

#include "Engine/GL/Frame.hpp"
#include "Engine/GL/Program.h"
#include "Labs/Common/ICase.h"
#include "Labs/Common/ImageRGB.h"
#include "Labs/Common/OrbitCameraManager.h"
#include "Labs/Photon_Mapping/Content.h"
#include "Labs/Photon_Mapping/Intersecter.h"
#include "Labs/Photon_Mapping/PhotonMapping.h"
#include "Labs/Photon_Mapping/SceneObject.h"

namespace VCX::Labs::Rendering {

    class CaseSimple : public Common::ICase {
    public:
        CaseSimple(const std::initializer_list<Assets::ExampleScene> & scenes);
        ~CaseSimple();

        virtual std::string_view const GetName() override { return "Simple Photon Mapping"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

    protected:
        std::vector<Assets::ExampleScene> const _scenes;
        Engine::GL::UniqueProgram               _program;
        Engine::GL::UniqueRenderFrame           _frame;
        Engine::GL::UniqueRenderItem            _pointItem;
        SceneObject                             _sceneObject;
        Common::OrbitCameraManager              _cameraManager;

        Engine::GL::UniqueTexture2D _texture;
        RayIntersector              _intersector;
        PhotonMapping               _photonmapping;
        std::vector<glm::vec3>      photon_pos;

        std::size_t      _sceneIdx { 0 };
        bool             _enableZoom { true };
        bool             _enableShadow { true };
        int              _maximumDepth { 30 };
        int              _photonPerLight { 100000 };
        int              _numNearPhoton { 200 };
        float            _photonProgress { 0.0f };
        int              _superSampleRate { 1 };
        std::size_t      _pixelIndex { 0 };
        bool             _onInit { false };
        bool             _stopFlag { true };
        bool             _sceneDirty { true };
        bool             _treeDirty { true };
        bool             _resetDirty { true };
        Common::ImageRGB _buffer;
        bool             _resizable { true };
        float            _gamma = 2.5f;

        std::thread _task;

        auto GetBufferSize() const { return std::pair(std::uint32_t(_buffer.GetSizeX()), std::uint32_t(_buffer.GetSizeY())); }

        char const *          GetSceneName(std::size_t const i) const { return Content::SceneNames[std::size_t(_scenes[i])].c_str(); }
        Engine::Scene const & GetScene(std::size_t const i) const { return Content::Scenes[std::size_t(_scenes[i])]; }
    };
} // namespace VCX::Labs::Rendering
