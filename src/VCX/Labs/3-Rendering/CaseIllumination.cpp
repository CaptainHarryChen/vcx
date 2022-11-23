#include <spdlog/spdlog.h>

#include "Labs/3-Rendering/CaseIllumination.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::Rendering {
    CaseIllumination::CaseIllumination(std::initializer_list<Assets::ExampleScene> && scenes) :
        _scenes(scenes),
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/phong.vert"),
                Engine::GL::SharedShader("assets/shaders/phong.frag") })),
        _sceneObject(0) {
        _cameraManager.AutoRotate = false;
        _program.BindUniformBlock("PassConstants", 0);
        _program.GetUniforms().SetByName("u_DiffuseMap" , 0);
        _program.GetUniforms().SetByName("u_SpecularMap", 1);
        _program.GetUniforms().SetByName("u_HeightMap"  , 2);
    }

    void CaseIllumination::OnSetupPropsUI() {
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
            _uniformDirty |= ImGui::RadioButton("Phong Model", &_useBlinn, 0);
            ImGui::SameLine();
            _uniformDirty |= ImGui::RadioButton("Blinn-Phong Model", &_useBlinn, 1);
            _uniformDirty |= ImGui::SliderFloat("Shininess", &_shininess, 1, 128, "%.1f", ImGuiSliderFlags_Logarithmic);
            _uniformDirty |= ImGui::SliderFloat("Ambient", &_ambientScale, 0.f, 2.f, "%.2fx");
            _uniformDirty |= ImGui::Checkbox("Gamma Correction", &_useGammaCorrection);
            _uniformDirty |= ImGui::SliderInt("Attenuation", &_attenuationOrder, 0, 2);
            _uniformDirty |= ImGui::SliderInt("Bump", &_bumpMappingPercent, 0, 100, "%d%%");
        }
        ImGui::Spacing();

		if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Ease Touch", &_cameraManager.EnableDamping);
        }
        ImGui::Spacing();
    }

    Common::CaseRenderResult CaseIllumination::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
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

            _program.GetUniforms().SetByName("u_AmbientScale"      , _ambientScale);
            _program.GetUniforms().SetByName("u_UseBlinn"          , _useBlinn);
            _program.GetUniforms().SetByName("u_Shininess"         , _shininess);
            _program.GetUniforms().SetByName("u_UseGammaCorrection", int(_useGammaCorrection));
            _program.GetUniforms().SetByName("u_AttenuationOrder"  , _attenuationOrder);            
            _program.GetUniforms().SetByName("u_BumpMappingBlend"  , _bumpMappingPercent * .01f);            
        }

        gl_using(_frame);

        glEnable(GL_DEPTH_TEST);

        for (auto const & model : _sceneObject.OpaqueModels) {
            auto const & material = _sceneObject.Materials[model.MaterialIndex];
            model.Mesh.Draw({ material.Albedo.Use(), material.MetaSpec.Use(), material.Height.Use(), _program.Use() });
        }

        glDisable(GL_DEPTH_TEST);

        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

    void CaseIllumination::OnProcessInput(ImVec2 const& pos) {
        _cameraManager.ProcessInput(_sceneObject.Camera, pos);
    }
}
