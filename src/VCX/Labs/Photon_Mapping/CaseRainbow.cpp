#include "Labs/Photon_Mapping/CaseRainbow.h"

namespace VCX::Labs::Rendering {

    static glm::vec3 RayTrace(const PhotonMapping & globalPhotonMapping, const PhotonMapping & causticPhotonMapping, const RayIntersector & intersector, Ray ray, int maxDepth, bool enableShadow, int numNearPhoton, int causticNumNearPhoton) {
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

        // Caculate direct light
        color += DirectLight(intersector, ray, rayHit, enableShadow);

        // culculate indirect light
        if (isDiffuse) {
            // caustic
            color += weight * causticPhotonMapping.CollatePhotons(rayHit, -ray.Direction, causticNumNearPhoton, 0.15f);
            // indirect diffuse
            color += weight * globalPhotonMapping.CollatePhotons(rayHit, -ray.Direction, numNearPhoton, 1.0f);
        }

        return color;
    }

    CaseRainbow::CaseRainbow(const std::initializer_list<Assets::ExampleScene> & scenes):
        CaseCaustic(scenes) {
        }

    CaseRainbow::~CaseRainbow() {
    }

    Common::CaseRenderResult CaseRainbow::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
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
                _globalPointItem.Draw({ _program.Use() });
                _program.GetUniforms().SetByName("u_Color", glm::vec3(0.0f, 1.0f, 0.0f));
                _causticPointItem.Draw({ _program.Use() });
            }
            glDisable(GL_DEPTH_TEST);
        }
        if (_onInit && ! _task.joinable()) {
            _task = std::thread([&]() {
                if (_pixelIndex == 0 && _treeDirty) {
                    Engine::Scene const & scene = GetScene(_sceneIdx);
                    _intersector.InitScene(&scene);
                    _globalPhotonMapping.onInit   = &_onInit;
                    _globalPhotonMapping.progress = &_photonProgress;
                    _globalPhotonMapping.InitScene(&scene, _intersector, false, _photonPerLight);
                    _causticPhotonMapping.progress = &_photonProgress;
                    _causticPhotonMapping.onInit   = &_onInit;
                    _causticPhotonMapping.InitCausticDispersion(&scene, _intersector, _causticPhotonPerLight);

                    if (! _onInit) return;
                    globalPhoton_pos.clear();
                    for (const auto & p : _globalPhotonMapping.photons)
                        globalPhoton_pos.push_back(p.Origin);
                    causticPhoton_pos.clear();
                    for (const auto & p : _causticPhotonMapping.photons)
                        causticPhoton_pos.push_back(p.Origin);
                    _treeDirty = false;
                }
            });
        }

        if (! _treeDirty) {
            if (_onInit) {
                if (_task.joinable())
                    _task.join();
                auto hist_span = std::span<const std::byte>(reinterpret_cast<const std::byte *>(globalPhoton_pos.data()), reinterpret_cast<const std::byte *>(globalPhoton_pos.data() + globalPhoton_pos.size()));
                _globalPointItem.UpdateVertexBuffer("position", hist_span);
                hist_span = std::span<const std::byte>(reinterpret_cast<const std::byte *>(causticPhoton_pos.data()), reinterpret_cast<const std::byte *>(causticPhoton_pos.data() + causticPhoton_pos.size()));
                _causticPointItem.UpdateVertexBuffer("position", hist_span);
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
                            glm::vec3 res = RayTrace(_globalPhotonMapping, _causticPhotonMapping, _intersector, initialRay, _maximumDepth, _enableShadow, _numNearPhoton, _causticNumNearPhoton);
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
} // namespace VCX::Labs::Rendering
