#include "Labs/Photon_Mapping/CaseSimple.h"

namespace VCX::Labs::Rendering {

    glm::vec3 RayTrace(const RayIntersector & intersector, Ray ray, int maxDepth, bool enableShadow) {
        glm::vec3 color(0.0f);
        glm::vec3 weight(1.0f);
        float     gamma   = 2.2f;
        float     ambient = 0.05f;

        for (int depth = 0; depth < maxDepth; depth++) {
            glm::vec3 pos, n, kd, ks;
            float     alpha, shininess;
            do {
                auto rayHit = intersector.IntersectRay(ray);
                if (! rayHit.IntersectState) return color;
                pos       = rayHit.IntersectPosition;
                n         = rayHit.IntersectNormal;
                kd        = rayHit.IntersectAlbedo;
                ks        = rayHit.IntersectMetaSpec;
                alpha     = rayHit.IntersectAlbedo.w;
                shininess = rayHit.IntersectMetaSpec.w * 256;
                if (alpha < .2)
                    ray.Origin = pos;
            } while (alpha < .2);

            glm::vec3 result(0.0f);
            /******************* 2. Whitted-style ray tracing *****************/
            // your code here
            result = kd * ambient;

            for (const Engine::Light & light : intersector.InternalScene->Lights) {
                glm::vec3 l;
                float     attenuation;
                /******************* 3. Shadow ray *****************/
                if (light.Type == Engine::LightType::Point) {
                    l           = light.Position - pos;
                    attenuation = 1.0f / glm::dot(l, l);
                    if (enableShadow) {
                        // your code here
                        auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
                        while(shadowRayHit.IntersectState && shadowRayHit.IntersectAlbedo.w < 0.2)
                            shadowRayHit = intersector.IntersectRay(Ray(shadowRayHit.IntersectPosition, glm::normalize(l)));
                        if (shadowRayHit.IntersectState) {
                            glm::vec3 sh = shadowRayHit.IntersectPosition - pos;
                            if (glm::dot(sh, sh) < glm::dot(l, l))
                                attenuation = 0.0f;
                        }
                    }
                } else if (light.Type == Engine::LightType::Directional) {
                    l           = light.Direction;
                    attenuation = 1.0f;
                    if (enableShadow) {
                        // your code here
                        auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
                        while(shadowRayHit.IntersectState && shadowRayHit.IntersectAlbedo.w < 0.2)
                            shadowRayHit = intersector.IntersectRay(Ray(shadowRayHit.IntersectPosition, glm::normalize(l)));
                        if (shadowRayHit.IntersectState)
                            attenuation = 0.0f;
                    }
                }

                /******************* 2. Whitted-style ray tracing *****************/
                // your code here
                glm::vec3 h         = glm::normalize(-ray.Direction + glm::normalize(l));
                float     spec_coef = glm::pow(glm::max(glm::dot(h, n), 0.0f), shininess);
                float     diff_coef = glm::max(glm::dot(glm::normalize(l), n), 0.0f);
                result += light.Intensity * attenuation * (diff_coef * kd + spec_coef * ks);
            }
            // result = pow(result, glm::vec3(1. / gamma));

            if (alpha < 0.9) {
                // refraction
                // accumulate color
                glm::vec3 R = alpha * glm::vec3(1.0f);
                color += weight * R * result;
                weight *= glm::vec3(1.0f) - R;

                // generate new ray
                ray = Ray(pos, ray.Direction);
            } else {
                // reflection
                // accumulate color
                glm::vec3 R = ks * glm::vec3(0.5f);
                color += weight * (glm::vec3(1.0f) - R) * result;
                weight *= R;

                // generate new ray
                glm::vec3 out_dir = ray.Direction - glm::vec3(2.0f) * n * glm::dot(n, ray.Direction);
                ray               = Ray(pos, out_dir);
            }
        }

        return color;
    }

    CaseSimple::CaseSimple(std::initializer_list<Assets::ExampleScene> && scenes):
        _scenes(scenes),
        _program(
            Engine::GL::UniqueProgram({ Engine::GL::SharedShader("assets/shaders/flat.vert"),
                                        Engine::GL::SharedShader("assets/shaders/flat.frag") })),
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
                        _sceneIdx   = i;
                        _sceneDirty = true;
                        _treeDirty  = true;
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
