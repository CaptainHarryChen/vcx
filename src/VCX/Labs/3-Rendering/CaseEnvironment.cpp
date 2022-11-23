#include <spdlog/spdlog.h>

#include "Labs/3-Rendering/CaseEnvironment.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Rendering {
    CaseEnvironment::CaseEnvironment(std::initializer_list<Assets::ExampleScene> && scenes) :
        _scenes(scenes),
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/envmap.vert"),
                Engine::GL::SharedShader("assets/shaders/envmap.frag") })),
        _skyboxProgram(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/skybox.vert"),
                Engine::GL::SharedShader("assets/shaders/skybox.frag") })),
        _sceneObject(2) {
        _cameraManager.AutoRotate = false;
        _program.BindUniformBlock("PassConstants", 2);
        _program.GetUniforms().SetByName("u_DiffuseMap" ,    0);
        _program.GetUniforms().SetByName("u_EnvironmentMap", 1);
        _skyboxProgram.BindUniformBlock("PassConstants", 2);
        _skyboxProgram.GetUniforms().SetByName("u_Skybox", 1);
    }

    void CaseEnvironment::OnSetupPropsUI() {
        if (ImGui::BeginCombo("Scene", GetSceneName(_sceneIdx))) {
            for (std::size_t i = 0; i < _scenes.size(); ++i) {
                bool selected = i == _sceneIdx;
                if (ImGui::Selectable(GetSceneName(i), selected)) {
                    if (! selected) {
                        _sceneIdx           = i;
                        _recompute          = true;
                        _uniformDirty = true;
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Combo("Anti-Aliasing", &_msaa, "None\0002x MSAA\0004x MSAA\0008x MSAA\0");
        Common::ImGuiHelper::SaveImage(_frame.GetColorAttachment(), _frame.GetSize(), true);
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
            _uniformDirty |= ImGui::SliderFloat("Ambient", &_ambientScale, 0.0f, 2.f, "%.2fx");
            _uniformDirty |= ImGui::SliderFloat("Diffuse", &_diffuseScale, 0.0f, 2.f, "%.2fx");
            _uniformDirty |= ImGui::SliderFloat("Environment", &_environmentScale, 0.0f, 2.f, "%.2fx");
        }
        ImGui::Spacing();

		if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Ease Touch", &_cameraManager.EnableDamping);
        }
        ImGui::Spacing();
    }

    Common::CaseRenderResult CaseEnvironment::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (_recompute) {
            _recompute = false;
            _sceneObject.ReplaceScene(GetScene(_sceneIdx));
            if (_sceneObject.Skybox.has_value())
                _sceneObject.Skybox.value().CubeMap.SetUnit(1);
            _cameraManager.Save(_sceneObject.Camera);
        }
        _frame.Resize(desiredSize, 1 << _msaa);

        _cameraManager.Update(_sceneObject.Camera);
        _sceneObject.PassConstantsBlock.Update(&SceneObject::PassConstants::Projection, _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _sceneObject.PassConstantsBlock.Update(&SceneObject::PassConstants::View, _sceneObject.Camera.GetViewMatrix());
        _sceneObject.PassConstantsBlock.Update(&SceneObject::PassConstants::ViewPosition, _sceneObject.Camera.Eye);

        if (_uniformDirty) {
            _uniformDirty = false;

            _program.GetUniforms().SetByName("u_AmbientScale"    , _ambientScale);
            _program.GetUniforms().SetByName("u_DiffuseScale"    , _diffuseScale);
            _program.GetUniforms().SetByName("u_EnvironmentScale", _environmentScale);
        }

        gl_using(_frame);

        glEnable(GL_DEPTH_TEST);

        auto const & skybox = _sceneObject.Skybox.value();

        for (auto const & model : _sceneObject.OpaqueModels) {
            auto const & material = _sceneObject.Materials[model.MaterialIndex];
            model.Mesh.Draw({ material.Albedo.Use(), skybox.CubeMap.Use(), _program.Use() });
        }

        glDepthFunc(GL_LEQUAL);
        skybox.Mesh.Draw({ skybox.CubeMap.Use(), _skyboxProgram.Use() });
        glDepthFunc(GL_LESS);

        glDisable(GL_DEPTH_TEST);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseEnvironment::OnProcessInput(ImVec2 const& pos) {
        _cameraManager.ProcessInput(_sceneObject.Camera, pos);
    }
}
