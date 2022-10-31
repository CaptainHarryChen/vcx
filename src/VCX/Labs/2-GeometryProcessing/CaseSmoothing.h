#pragma once

#include "Engine/Async.hpp"
#include "Labs/2-GeometryProcessing/Viewer.h"
#include "Labs/Common/OrbitCameraManager.h"

namespace VCX::Labs::GeometryProcessing {

    class CaseSmoothing : public Common::ICase {
    public:
        CaseSmoothing(Viewer & viewer, std::initializer_list<Assets::ExampleModel> && models);

        virtual std::string_view const GetName() override { return "Mesh Smoothing"; }

        virtual void                     OnSetupPropsUI() override;
        virtual Common::CaseRenderResult OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) override;
        virtual void                     OnProcessInput(ImVec2 const & pos) override;

    private:
        std::vector<Assets::ExampleModel> const _models;

        Engine::Async<Engine::SurfaceMesh>    _task;
        Viewer                             &  _viewer;
        Engine::Camera                        _camera           { .Eye = glm::vec3(-1, 1, 1) };
        Common::OrbitCameraManager            _cameraManager    { glm::vec3(-1, 1, 1) };
        std::size_t                           _modelIdx         { 0 };
        bool                                  _recompute        { true };
        bool                                  _running          { false };
        ModelObject                           _modelObject;
        RenderOptions                         _options;
        int                                   _numIterations    { 5 };
        float                                 _lambda           { .9f };
        int                                   _useUniformWeight { 1 };

        char const *                GetModelName(std::size_t const i) const { return Viewer::ExampleModelNames[std::size_t(_models[i])].c_str(); }
        Engine::SurfaceMesh const & GetModelMesh(std::size_t const i) const { return Viewer::ExampleModelMeshes[std::size_t(_models[i])]; }
    };
} // namespace VCX::Labs::GeometryProcessing
