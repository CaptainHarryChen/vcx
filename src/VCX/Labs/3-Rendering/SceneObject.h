#pragma once

#include "Engine/GL/Program.h"
#include "Engine/GL/RenderItem.h"
#include "Engine/GL/Texture.hpp"
#include "Engine/GL/UniformBlock.hpp"
#include "Engine/Scene.h"

namespace VCX::Labs::Rendering {
    struct SkyboxObject {
        Engine::GL::UniqueRenderItem     Mesh;
        Engine::GL::UniqueTextureCubeMap CubeMap;

        explicit SkyboxObject(Engine::Skybox const & skybox);
    };

    struct MaterialObject {
        Engine::GL::UniqueTexture2D Albedo;
        Engine::GL::UniqueTexture2D MetaSpec;
        Engine::GL::UniqueTexture2D Height;

        explicit MaterialObject(Engine::Material const & material);
    };

    struct ModelObject {
        Engine::GL::UniqueIndexedRenderItem Mesh;
        std::uint32_t                       MaterialIndex;

        explicit ModelObject(Engine::Model const & model);
    };

    struct SceneObject {
        static constexpr std::size_t c_MaxCntLights = 4;

        struct alignas(16) Light {
            alignas(16) glm::vec3 Intensity  { 1, 1, 1 };
            alignas(16) glm::vec3 Direction  { 1, 0, 0 };
            alignas(16) glm::vec3 Position   { 0, 0, 0 };
            alignas(4)  float     CutOff     { 1 };
            alignas(4)  float     OuterCutOff { 0 };
        };

        struct PassConstants {
            alignas(64) glm::mat4          Projection;
            alignas(64) glm::mat4          View;
            alignas(16) glm::vec3          ViewPosition;
            alignas(16) glm::vec3          AmbientIntensity;
            alignas(16) SceneObject::Light Lights[c_MaxCntLights];
            alignas(16) int                CntPointLights; // Fix alignment for clang OSX: alignas(4) -> alignas(16)
            alignas(4)  int                CntSpotLights;
            alignas(4)  int                CntDirectionalLights;
        };

        Engine::ReflectionType       Reflection;
        glm::vec3                    AmbientIntensity;
        std::optional<SkyboxObject>  Skybox;

        Engine::Camera               Camera;
        
        std::vector<Light>           Lights;
        std::size_t                  CntPointLights;
        std::size_t                  CntSpotLights;
        std::size_t                  CntDirectionalLights;

        std::vector<MaterialObject>  Materials;

        std::vector<ModelObject>     OpaqueModels;
        std::vector<ModelObject>     TransparentModels;

        Engine::GL::UniqueUniformBlock<PassConstants> PassConstantsBlock;

        SceneObject(int const bindingPoint) : PassConstantsBlock(bindingPoint, Engine::GL::DrawFrequency::Stream) { }

        void ReplaceScene(Engine::Scene const & scene);
    };
} // namespace VCX::Labs::Rendering