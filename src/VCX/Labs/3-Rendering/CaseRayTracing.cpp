#include "Labs/3-Rendering/CaseRayTracing.h"

namespace VCX::Labs::Rendering {

    CaseRayTracing::CaseRayTracing(std::initializer_list<Assets::ExampleScene> && scenes):
        _scenes(scenes),
        _program(
            Engine::GL::UniqueProgram({
                Engine::GL::SharedShader("assets/shaders/flat.vert"),
                Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _sceneObject(4),
        _texture({ .MinFilter = Engine::GL::FilterMode::Linear, .MagFilter = Engine::GL::FilterMode::Nearest }) {
        _cameraManager.AutoRotate = false;
        _program.GetUniforms().SetByName("u_Color", glm::vec3(1, 1, 1));
    }

    CaseRayTracing::~CaseRayTracing() {
        _stopFlag = true;
        if (_task.joinable()) _task.join();
    }

    void CaseRayTracing::OnSetupPropsUI() {
        if (ImGui::BeginCombo("Scene", GetSceneName(_sceneIdx))) {
            for (std::size_t i = 0; i < _scenes.size(); ++i) {
                bool selected = i == _sceneIdx;
                if (ImGui::Selectable(GetSceneName(i), selected)) {
                    if (! selected) {
                        _sceneIdx   = i;
                        _sceneDirty = true;
                        _treeDirty = true;
                        _resetDirty = true;
                    }
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Reset Scene")) _resetDirty = true;
        ImGui::SameLine();
        if (_task.joinable()) {
            if (ImGui::Button("Stop Rendering")) {
                _stopFlag = true;
                if (_task.joinable()) _task.join();
            }
        } else if (ImGui::Button("Start Rendering")) _stopFlag = false;
        ImGui::ProgressBar(float(_pixelIndex) / (_buffer.GetSizeX() * _buffer.GetSizeY()));
        Common::ImGuiHelper::SaveImage(_texture, GetBufferSize(), true);
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
            _resetDirty |= ImGui::SliderInt("Sample Rate", &_superSampleRate, 1, 5);
            _resetDirty |= ImGui::SliderInt("Max Depth", &_maximumDepth, 1, 15);
            _resetDirty |= ImGui::Checkbox("Shadow Ray", &_enableShadow);
        }
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        }
        ImGui::Spacing();
    }

    Common::CaseRenderResult CaseRayTracing::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
        if (_resetDirty) {
            _stopFlag = true;
            if (_task.joinable()) _task.join();
            _pixelIndex = 0;
            _resizable  = true;
            _resetDirty = false;
        }
        if (_sceneDirty) {
            _sceneObject.ReplaceScene(GetScene(_sceneIdx));
            _cameraManager.Save(_sceneObject.Camera);
            _sceneDirty = false;
        }
        if (_resizable) {
            _frame.Resize(desiredSize);
            _cameraManager.Update(_sceneObject.Camera);
            _program.GetUniforms().SetByName("u_Projection", _sceneObject.Camera.GetProjectionMatrix((float(desiredSize.first) / desiredSize.second)));
            _program.GetUniforms().SetByName("u_View"      , _sceneObject.Camera.GetViewMatrix());
            
            gl_using(_frame);

            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            for (auto const & model : _sceneObject.OpaqueModels)
                model.Mesh.Draw({ _program.Use() });
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_DEPTH_TEST);
        }
        if (! _stopFlag && ! _task.joinable()) {
            if (_pixelIndex == 0) {
                _resizable = false;
                _buffer    = _frame.GetColorAttachment().Download<Engine::Formats::RGB8>();
            }
            _task = std::thread([&]() {
                auto const width  = _buffer.GetSizeX();
                auto const height = _buffer.GetSizeY();
                if (_pixelIndex == 0 && _treeDirty) {
                    Engine::Scene const & scene = GetScene(_sceneIdx);
                    _intersector.InitScene(&scene);
                    _treeDirty = false;
                }
                // Render into tex.
                while (_pixelIndex < std::size_t(width) * height) {
                    int i = _pixelIndex % width;
                    int j = _pixelIndex / width;
                    glm::vec3 sum(0.0f);
                    for (int dy = 0; dy < _superSampleRate; ++dy)
                        for (int dx = 0; dx < _superSampleRate; ++dx) {
                            float        step = 1.0f / _superSampleRate;
                            float        di = step * (0.5f + dx), dj = step * (0.5f + dy);
                            auto const & camera    = _sceneObject.Camera;
                            glm::vec3    lookDir   = glm::normalize(camera.Target - camera.Eye);
                            glm::vec3    rightDir  = glm::normalize(glm::cross(lookDir, camera.Up));
                            glm::vec3    upDir     = glm::normalize(glm::cross(rightDir, lookDir));
                            float const  aspect    = width * 1.f / height;
                            float const  fovFactor = std::tan(glm::radians(camera.Fovy) / 2);
                            lookDir += fovFactor * (2.0f * (j + dj) / height - 1.0f) * upDir;
                            lookDir += fovFactor * aspect * (2.0f * (i + di) / width - 1.0f) * rightDir;
                            Ray       initialRay(camera.Eye, glm::normalize(lookDir));
                            glm::vec3 res = RayTrace(_intersector, initialRay, _maximumDepth, _enableShadow);
                            sum += glm::pow(res, glm::vec3(1.0 / 2.2));
                        }
                    _buffer.At(i, j) = sum / glm::vec3(_superSampleRate * _superSampleRate);
                    ++_pixelIndex;
                    if (_stopFlag) return;
                }
            });
        }
        if (! _resizable) {
            if (!_stopFlag) _texture.Update(_buffer);
            if (_task.joinable() && _pixelIndex == _buffer.GetSizeX() * _buffer.GetSizeY()) {
                _stopFlag = true;
                _task.join();
            }
        }
        return Common::CaseRenderResult {
            .Fixed     = false,
            .Flipped   = true,
            .Image     = _resizable ? _frame.GetColorAttachment() : _texture,
            .ImageSize = _resizable ? desiredSize : GetBufferSize(),
        };
    }

    void CaseRayTracing::OnProcessInput(ImVec2 const & pos) {
        auto         window  = ImGui::GetCurrentWindow();
        bool         hovered = false;
        bool         anyHeld = false;
        ImVec2 const delta   = ImGui::GetIO().MouseDelta;
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##io"), &hovered, &anyHeld);
        if (! hovered) return;
        if (_resizable) {
            _cameraManager.ProcessInput(_sceneObject.Camera, pos);
        } else {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && delta.x != 0.f)
                ImGui::SetScrollX(window, window->Scroll.x - delta.x);
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && delta.y != 0.f)
                ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        }
        if (_enableZoom && ! anyHeld && ImGui::IsItemHovered())
            Common::ImGuiHelper::ZoomTooltip(_resizable ? _frame.GetColorAttachment() : _texture, GetBufferSize(), pos, true);
    }

} // namespace VCX::Labs::Rendering
