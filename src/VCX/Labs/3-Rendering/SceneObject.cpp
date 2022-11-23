#pragma once

#include <spdlog/spdlog.h>

#include "Labs/3-Rendering/SceneObject.h"

namespace VCX::Labs::Rendering {
    SkyboxObject::SkyboxObject(Engine::Skybox const & skybox) :
        Mesh(Engine::GL::VertexLayout().Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0)),
        CubeMap(skybox.Images, {
            .WrapU     = Engine::GL::WrapMode::Clamp,
            .WrapV     = Engine::GL::WrapMode::Clamp,
            .WrapW     = Engine::GL::WrapMode::Clamp,
            .MinFilter = Engine::GL::FilterMode::Linear,
            .MagFilter = Engine::GL::FilterMode::Linear 
        }) {
        Mesh.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(skybox.c_PositionData));
    }

    template<Engine::TextureFormat Format>
    static Engine::GL::UniqueTexture2D MakeTexture(Engine::Texture2D<Format> const & val, std::uint32_t const texUnit) {
        return Engine::GL::UniqueTexture2D(
            val, {
                .WrapU     = Engine::GL::WrapMode::Repeat,
                .WrapV     = Engine::GL::WrapMode::Repeat,
                .MinFilter = Engine::GL::FilterMode::Trilinear,
                .MagFilter = Engine::GL::FilterMode::Trilinear },
            texUnit);
    }

    MaterialObject::MaterialObject(Engine::Material const & material) :
        Albedo(MakeTexture(material.Albedo, 0)),
        MetaSpec(MakeTexture(material.MetaSpec, 1)),
        Height(MakeTexture(material.Height, 2)) {
    }

    static Engine::GL::UniqueIndexedRenderItem MakeRenderItem(Engine::SurfaceMesh const & mesh) {
        Engine::GL::UniqueIndexedRenderItem item(Engine::GL::VertexLayout()
                .Add<glm::vec3>("position", Engine::GL::DrawFrequency::Static, 0)
                .Add<glm::vec3>("normal", Engine::GL::DrawFrequency::Static, 1)
                .Add<glm::vec2>("texcoord", Engine::GL::DrawFrequency::Static, 2),
            Engine::GL::PrimitiveType::Triangles);
        item.UpdateVertexBuffer("position", Engine::make_span_bytes<glm::vec3>(mesh.Positions));
        item.UpdateVertexBuffer("normal", Engine::make_span_bytes<glm::vec3>(mesh.IsNormalAvailable() ? mesh.Normals : mesh.ComputeNormals()));
        item.UpdateVertexBuffer("texcoord", Engine::make_span_bytes<glm::vec2>(mesh.IsTexCoordAvailable() ? mesh.TexCoords : mesh.GetEmptyTexCoords()));
        item.UpdateElementBuffer(mesh.Indices);
        return item;
    }

    ModelObject::ModelObject(Engine::Model const & model) :
        Mesh(MakeRenderItem(model.Mesh)), 
        MaterialIndex(model.MaterialIndex) {
    }

    void SceneObject::ReplaceScene(Engine::Scene const & scene) {
        Reflection       = scene.Reflection;
        AmbientIntensity = scene.AmbientIntensity;
        Camera           = scene.Cameras[0];

        if (! scene.Skyboxes.empty()) {
            Skybox.emplace(scene.Skyboxes[0]);
        } else {
            Skybox.reset();
        }

        std::vector<Light> pointLights;
        std::vector<Light> spotLights;
        std::vector<Light> directionalLights;
        for (auto const & light : scene.Lights) {
            auto val = Light {
                .Intensity   = light.Intensity,
                .Direction   = glm::normalize(light.Direction),
                .Position    = light.Position,
                .CutOff      = light.CutOff,
                .OuterCutOff = light.OuterCutOff,
            };
            if (light.Type == Engine::LightType::Point) {
                pointLights.push_back(std::move(val));
            } else if (light.Type == Engine::LightType::Spot) {
                spotLights.push_back(std::move(val));
            } else if (light.Type == Engine::LightType::Directional) {
                directionalLights.push_back(std::move(val));
            }
        }
        Lights.clear();
        Lights.insert(Lights.end(), pointLights.begin(), pointLights.end());
        Lights.insert(Lights.end(), spotLights.begin(), spotLights.end());
        Lights.insert(Lights.end(), directionalLights.begin(), directionalLights.end());
        CntPointLights       = pointLights.size();
        CntSpotLights        = spotLights.size();
        CntDirectionalLights = directionalLights.size();

        Materials.clear();
        for (auto const & material : scene.Materials)
            Materials.push_back(MaterialObject(material));
        
        OpaqueModels.clear();
        TransparentModels.clear();
        for (auto const & model : scene.Models) {
            auto const blend = scene.Materials[model.MaterialIndex].Blend;
            if (blend == Engine::BlendMode::Opaque) {
                OpaqueModels.push_back(ModelObject(model));
            } else if (blend == Engine::BlendMode::Transparent) {
                TransparentModels.push_back(ModelObject(model));
            }
        }

        auto passConstants = PassConstants {
            .AmbientIntensity   = AmbientIntensity,
        };
        std::copy(Lights.data(), Lights.data() + std::min(Lights.size(), c_MaxCntLights), passConstants.Lights);
        passConstants.CntPointLights       = int(std::min(CntPointLights,        c_MaxCntLights));
        passConstants.CntSpotLights        = int(std::min(CntSpotLights,         c_MaxCntLights - passConstants.CntPointLights));
        passConstants.CntDirectionalLights = int(std::min(CntDirectionalLights,  c_MaxCntLights - passConstants.CntPointLights - passConstants.CntSpotLights));
        PassConstantsBlock.Update(passConstants);
    }
} // namespace VCX::Labs::Rendering