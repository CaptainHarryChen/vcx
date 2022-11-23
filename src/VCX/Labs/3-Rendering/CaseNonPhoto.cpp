#include <spdlog/spdlog.h>

#include "Labs/3-Rendering/CaseNonPhoto.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Rendering {
    CaseNonPhoto::CaseNonPhoto(std::initializer_list<Assets::ExampleScene> && scenes) :
        _scenes(scenes),
        _backLineProgram(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/npr-line.vert"), 
                Engine::GL::SharedShader("assets/shaders/npr-line.frag")})),
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/npr.vert"),
                Engine::GL::SharedShader("assets/shaders/npr.frag") })),
        _sceneObject(1) {
        _cameraManager.AutoRotate = false;
        _backLineProgram.BindUniformBlock("PassConstants", 1);
        _backLineProgram.GetUniforms().SetByName("u_LineWidth", _lineWidth);
        _program.BindUniformBlock("PassConstants", 1);
        _program.GetUniforms().SetByName("u_CoolColor", _coolColor);
        _program.GetUniforms().SetByName("u_WarmColor", _warmColor);
    }

    void CaseNonPhoto::OnSetupPropsUI() {
        if (ImGui::BeginCombo("Scene", GetSceneName(_sceneIdx))) {
            for (std::size_t i = 0; i < _scenes.size(); ++i) {
                bool selected = i == _sceneIdx;
                if (ImGui::Selectable(GetSceneName(i), selected)) {
                    if (! selected) {
                        _sceneIdx     = i;
                        _recompute    = true;
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
            _uniformDirty |= ImGui::SliderFloat("Line Width", &_lineWidth, 1, 6);
            _uniformDirty |= ImGui::ColorEdit3("Cool Color", glm::value_ptr(_coolColor));
            _uniformDirty |= ImGui::ColorEdit3("Warm Color", glm::value_ptr(_warmColor));
        }
        ImGui::Spacing();

		if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Ease Touch", &_cameraManager.EnableDamping);
        }
        ImGui::Spacing();
    }

    Common::CaseRenderResult CaseNonPhoto::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (_recompute) {
            _recompute = false;
            _sceneObject.ReplaceScene(GetScene(_sceneIdx));
            _cameraManager.Save(_sceneObject.Camera);
        }
        _frame.Resize(desiredSize, 1 << _msaa);

        _cameraManager.Update(_sceneObject.Camera);
        _sceneObject.PassConstantsBlock.Update(&SceneObject::PassConstants::Projection, _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
        _sceneObject.PassConstantsBlock.Update(&SceneObject::PassConstants::View, _sceneObject.Camera.GetViewMatrix());
        _sceneObject.PassConstantsBlock.Update(&SceneObject::PassConstants::ViewPosition, _sceneObject.Camera.Eye);

        if (_uniformDirty) {
            _uniformDirty = false;
            _backLineProgram.GetUniforms().SetByName("u_ScreenWidth", int(desiredSize.first));
            _backLineProgram.GetUniforms().SetByName("u_ScreenHeight", int(desiredSize.second));
            _backLineProgram.GetUniforms().SetByName("u_LineWidth", _lineWidth);
            _program.GetUniforms().SetByName("u_CoolColor", _coolColor);
            _program.GetUniforms().SetByName("u_WarmColor", _warmColor);
        }

        gl_using(_frame);

        glCullFace(GL_FRONT);
        glEnable(GL_CULL_FACE);
        for (auto const & model : _sceneObject.OpaqueModels) {
            auto const & material = _sceneObject.Materials[model.MaterialIndex];
            model.Mesh.Draw({ _backLineProgram.Use() });
        }
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        for (auto const & model : _sceneObject.OpaqueModels) {
            auto const & material = _sceneObject.Materials[model.MaterialIndex];
            model.Mesh.Draw({ material.Albedo.Use(), material.MetaSpec.Use(), _program.Use() });
        }

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseNonPhoto::OnProcessInput(ImVec2 const& pos) {
        _cameraManager.ProcessInput(_sceneObject.Camera, pos);
    }
}
