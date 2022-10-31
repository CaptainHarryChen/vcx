#include <filesystem>

#include <spdlog/spdlog.h>

#include "Engine/loader.h"
#include "Labs/2-GeometryProcessing/Viewer.h"
#include "Labs/Common/ImGuiHelper.h"

namespace VCX::Labs::GeometryProcessing {
    static std::array<Engine::SurfaceMesh, Assets::ExampleModels.size()> LoadExampleModels() {
        std::array<Engine::SurfaceMesh, Assets::ExampleModels.size()> models;
        for (std::size_t i = 0; i < models.size(); i++) {
            models[i] = Engine::LoadSurfaceMesh(Assets::ExampleModels[i], true);
            models[i].NormalizePositions();
        }
        return models;
    }

    static std::array<std::string, Assets::ExampleModels.size()> LoadExampleNames() {
        std::array<std::string, Assets::ExampleModels.size()> names;
        for (std::size_t i = 0; i < names.size(); i++) {
            names[i] = std::filesystem::path(Assets::ExampleModels[i]).filename().string();
        }
        return names;
    }

    Viewer::Viewer():
        _passConstants({}),
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/three.vert"),
                Engine::GL::SharedShader("assets/shaders/three.geom"),
                Engine::GL::SharedShader("assets/shaders/three.frag") })),
        _uniformBlock(0, Engine::GL::DrawFrequency::Stream) {
        _program.BindUniformBlock("PassConstants", 0);
    }

    Common::CaseRenderResult Viewer::Render(RenderOptions const & options, ModelObject & modelObject, Engine::Camera &camera, Engine::ICameraManager & cameraManager, std::pair<std::uint32_t, std::uint32_t> desiredSize) {
        _frame.Resize(desiredSize);
        gl_using(_frame);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);

        cameraManager.Update(camera);

        _passConstants.Ambient          = options.Ambient;
        _passConstants.LightColor       = options.LightColor;
        _passConstants.LightDirection   = glm::normalize(options.LightDirection);
        _passConstants.ObjectColor      = options.ObjectColor;
        _passConstants.HasTexCoord      = modelObject.IsTexCoordAvailable();
        _passConstants.Wireframe        = options.Wireframe;
        _passConstants.Flat             = options.Flat;
        glm::mat4 model                 = modelObject.GetTransform();
        glm::mat4 view                  = camera.GetViewMatrix();
        _passConstants.NormalTransform  = glm::transpose(glm::inverse(model));
        _passConstants.Model            = model;
        _passConstants.Projection       = camera.GetProjectionMatrix(float(desiredSize.first) / desiredSize.second);
        _passConstants.View             = view;

        _uniformBlock.Update(_passConstants);

        if (options.Wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glEnable(GL_CULL_FACE);
        }

        modelObject.Draw({ _program.Use() });

        if (options.Wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glDisable(GL_CULL_FACE);
        }

        glDisable(GL_DEPTH_TEST);

        return {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _frame.GetColorAttachment(),
            .ImageSize = desiredSize,
        };
    }

	void Viewer::SetupRenderOptionsUI(RenderOptions & options, Common::OrbitCameraManager & cameraManager) {
		if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Ease Touch", &cameraManager.EnableDamping);
            ImGui::SliderFloat("Spin Speed", &cameraManager.AutoRotateSpeed, 0.0f, 20.0f, "%.0f");
        }
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Appearance")) {
            ImGui::Checkbox("Wireframes", &options.Wireframe);
            ImGui::ColorEdit3("Color", glm::value_ptr(options.ObjectColor));
            if (! options.Wireframe) {
                ImGui::Checkbox("Use Face Normal", &options.Flat);
                ImGui::SliderFloat("Ambient", &options.Ambient, 0.0f, 0.1f, "%.2f");
                ImGui::SliderFloat("Light", &options.LightDirScalar, 0.0f, 360.0f, "%.0f deg");
                options.LightDirection = glm::vec3(glm::cos(glm::radians(options.LightDirScalar)), -1.0f, glm::sin(glm::radians(options.LightDirScalar)));
            }
        }
        ImGui::Spacing();
	}

    std::array<Engine::SurfaceMesh, Assets::ExampleModels.size()> const Viewer::ExampleModelMeshes = LoadExampleModels();
    std::array<std::string,         Assets::ExampleModels.size()> const Viewer::ExampleModelNames  = LoadExampleNames();
} // namespace VCX::Labs::Common
