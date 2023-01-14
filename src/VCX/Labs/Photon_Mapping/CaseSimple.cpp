#include "Labs/Photon_Mapping/CaseSimple.h"

namespace VCX::Labs::Rendering {

    static glm::vec3 RayTrace(const PhotonMapping & photonMapping, const RayIntersector & intersector, Ray ray, int maxDepth, bool enableShadow, int numNearPhoton) {
        glm::vec3 color(0.0f);
        glm::vec3 weight(1.0f);

        bool   isDiffuse = false;
        RayHit rayHit;
        for (int depth = 0; depth < maxDepth; depth++) {
            rayHit = intersector.IntersectRay(ray);
            if (! rayHit.IntersectState)
                return color;
            if (rayHit.IntersectMode == Engine::BlendMode::Phong) {
                isDiffuse = true;
                break;
            }
            RayReflect rayReflect = DirectionFromBSDF(ray, rayHit);
            if (rayReflect.Type == ReflectType::Set)
                return rayReflect.Attenuation;
            weight *= rayReflect.Attenuation;
            ray.Origin    = rayHit.IntersectPosition;
            ray.Direction = rayReflect.Direction;
        }

        if (isDiffuse) {
            color += weight * photonMapping.CollatePhotons(rayHit, -ray.Direction, numNearPhoton);
        }

        return color;
    }

    CaseSimple::CaseSimple(const std::initializer_list<Assets::ExampleScene> & scenes):
        _scenes(scenes),
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })),
        _pointItem(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Stream, 0), Engine::GL::PrimitiveType::Points),
        _sceneObject(4),
        _texture({ .MinFilter = Engine::GL::FilterMode::Linear, .MagFilter = Engine::GL::FilterMode::Nearest }) {
        _cameraManager.AutoRotate = false;
        _program.GetUniforms().SetByName("u_Color", glm::vec3(1, 1, 1));
    }

    CaseSimple::~CaseSimple() {
        _stopFlag = true;
        if (_task.joinable()) _task.join();
    }

    void CaseSimple::OnSetupPropsUI() {
        if (ImGui::BeginCombo("Scene", GetSceneName(_sceneIdx))) {
            for (std::size_t i = 0; i < _scenes.size(); ++i) {
                bool selected = i == _sceneIdx;
                if (ImGui::Selectable(GetSceneName(i), selected)) {
                    if (! selected) {
                        _sceneIdx       = i;
                        _sceneDirty     = true;
                        _treeDirty      = true;
                        _resetDirty     = true;
                        _onInit         = false;
                        _stopFlag       = true;
                        if(_task.joinable()) _task.join();
                        _photonProgress = 0.0f;
                    }
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Reset Scene")) _resetDirty = true;
        if (ImGui::Button("Initialize")) {
            if (_treeDirty)
                _onInit = true;
        }
        ImGui::ProgressBar(_photonProgress);
        if (_task.joinable()) {
            if (_onInit) {
                if (ImGui::Button("Stop")) {
                    _onInit = false;
                    if (_task.joinable()) _task.join();
                    _treeDirty = true;
                }
            } else {
                if (ImGui::Button("Stop")) {
                    _stopFlag = true;
                    if (_task.joinable()) _task.join();
                }
            }
        } else if (ImGui::Button("Start Rendering")) {
            if (! _treeDirty)
                _stopFlag = false;
        }
        ImGui::ProgressBar(float(_pixelIndex) / (_buffer.GetSizeX() * _buffer.GetSizeY()));
        Common::ImGuiHelper::SaveImage(_texture, GetBufferSize(), true);
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool tmp = false;
            tmp |= ImGui::SliderInt("Photon per Light", &_photonPerLight, 100000, 1000000);
            _resetDirty |= tmp;
            _treeDirty |= tmp;
            _resetDirty |= ImGui::SliderInt("Nearest K Photon", &_numNearPhoton, 1, 1000);
            _resetDirty |= ImGui::SliderInt("Sample Rate", &_superSampleRate, 1, 5);
            _resetDirty |= ImGui::SliderInt("Max Depth", &_maximumDepth, 1, 15);
            _resetDirty |= ImGui::SliderFloat("Gamma", &_gamma, 1.0f, 10.0f);
            _resetDirty |= ImGui::Checkbox("Shadow Ray", &_enableShadow);
        }
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Control")) {
            ImGui::Checkbox("Zoom Tooltip", &_enableZoom);
        }
        ImGui::Spacing();
    }

    Common::CaseRenderResult CaseSimple::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
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
            _program.GetUniforms().SetByName("u_View", _sceneObject.Camera.GetViewMatrix());

            gl_using(_frame);

            _program.GetUniforms().SetByName("u_Color", glm::vec3(1.0f, 1.0f, 1.0f));
            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            for (auto const & model : _sceneObject.OpaqueModels)
                model.Mesh.Draw({ _program.Use() });
            for (auto const & model : _sceneObject.TransparentModels)
                model.Mesh.Draw({ _program.Use() });
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if (! _treeDirty) {
                glPointSize(1.f);
                _program.GetUniforms().SetByName("u_Color", glm::vec3(1.0f, 0.0f, 0.0f));
                _pointItem.Draw({ _program.Use() });
            }
            glDisable(GL_DEPTH_TEST);
        }
        if (_onInit && ! _task.joinable()) {
            _task = std::thread([&]() {
                if (_pixelIndex == 0 && _treeDirty) {
                    Engine::Scene const & scene = GetScene(_sceneIdx);
                    _intersector.InitScene(&scene);
                    _photonmapping.onInit = &_onInit;
                    _photonmapping.progress = &_photonProgress;
                    _photonmapping.InitScene(&scene, _intersector, true, _photonPerLight);
                    if(!_onInit) return;
                    photon_pos.clear();
                    for (const auto & p : _photonmapping.photons)
                        photon_pos.push_back(p.Origin);
                    _treeDirty = false;
                }
            });
        }

        if (! _treeDirty) {
            if (_onInit) {
                if (_task.joinable())
                    _task.join();
                auto hist_span = std::span<const std::byte>(reinterpret_cast<const std::byte *>(photon_pos.data()), reinterpret_cast<const std::byte *>(photon_pos.data() + photon_pos.size()));
                _pointItem.UpdateVertexBuffer("position", hist_span);
                _onInit = false;
            }
        }

        if (! _stopFlag && ! _treeDirty && ! _task.joinable()) {
            if (_pixelIndex == 0) {
                _resizable = false;
                _buffer    = _frame.GetColorAttachment().Download<Engine::Formats::RGB8>();
            }
            _task = std::thread([&]() {
                auto const width  = _buffer.GetSizeX();
                auto const height = _buffer.GetSizeY();

                // Render into tex.
                while (_pixelIndex < std::size_t(width) * height) {
                    int       i = _pixelIndex % width;
                    int       j = _pixelIndex / width;
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
                            glm::vec3 res = RayTrace(_photonmapping, _intersector, initialRay, _maximumDepth, _enableShadow, _numNearPhoton);
                            sum += glm::pow(res, glm::vec3(1.0 / _gamma));
                        }
                    _buffer.At(i, j) = sum / glm::vec3(_superSampleRate * _superSampleRate);
                    ++_pixelIndex;
                    if (_stopFlag) return;
                }
            });
        }
        if (! _resizable) {
            if (! _stopFlag) _texture.Update(_buffer);
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

    void CaseSimple::OnProcessInput(ImVec2 const & pos) {
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
