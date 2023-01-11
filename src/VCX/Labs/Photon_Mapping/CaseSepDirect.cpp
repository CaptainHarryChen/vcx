#include "Labs/Photon_Mapping/CaseSepDirect.h"

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
            weight *= rayReflect.Attenuation;
            ray.Origin    = rayHit.IntersectPosition;
            ray.Direction = rayReflect.Direction;
        }

        // Caculate dirrect light
        glm::vec3 pos, n, kd, ks;
        float     alpha, shininess;
        pos       = rayHit.IntersectPosition;
        n         = rayHit.IntersectNormal;
        kd        = rayHit.IntersectAlbedo;
        ks        = rayHit.IntersectMetaSpec;
        alpha     = rayHit.IntersectAlbedo.w;
        shininess = rayHit.IntersectMetaSpec.w * 256;

        for (const Engine::Light & light : intersector.InternalScene->Lights) {
            glm::vec3 l;
            float     attenuation;
            if (light.Type == Engine::LightType::Point) {
                l           = light.Position - pos;
                attenuation = 1.0f / glm::dot(l, l) / 4.0f / glm::pi<float>();
                if (enableShadow) {
                    auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
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
                    auto shadowRayHit = intersector.IntersectRay(Ray(pos, glm::normalize(l)));
                    if (shadowRayHit.IntersectState)
                        attenuation = 0.0f;
                }
            }
            glm::vec3 h         = glm::normalize(-ray.Direction + glm::normalize(l));
            float     spec_coef = glm::pow(glm::max(glm::dot(h, n), 0.0f), shininess);
            float     diff_coef = glm::max(glm::dot(glm::normalize(l), n), 0.0f);
            color += light.Intensity * attenuation * (diff_coef * kd + spec_coef * ks);
        }

        // culculate indirect light
        if (isDiffuse)
            color += weight * photonMapping.CollatePhotons(rayHit, -ray.Direction, numNearPhoton);

        return color;
    }

    CaseSepDirect::CaseSepDirect(const std::initializer_list<Assets::ExampleScene> & scenes):
        CaseSimple(scenes) {
    }

    CaseSepDirect::~CaseSepDirect() {
    }

    Common::CaseRenderResult CaseSepDirect::OnRender(std::pair<std::uint32_t, std::uint32_t> const desiredSize) {
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
                    _photonmapping.InitScene(&scene, _intersector, false);

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
} // namespace VCX::Labs::Rendering
