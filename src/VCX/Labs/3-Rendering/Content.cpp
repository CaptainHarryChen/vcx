#include <filesystem>

#include <spdlog/spdlog.h>

#include "Engine/loader.h"
#include "Labs/3-Rendering/Content.h"

namespace VCX::Labs::Rendering {
    static void AddGround(Engine::Scene & scene) {
        float const radius = scene.Cameras[0].ZFar * .5f;
        float height = std::numeric_limits<float>::max();
        for (auto const & model : scene.Models)
            height = std::min(height, model.Mesh.GetAxisAlignedBoundingBox().first.y);
        scene.Models.emplace_back();
        scene.Models.back().MaterialIndex = std::uint32_t(scene.Materials.size());
        scene.Models.back().Mesh.Positions = {
            { +radius, height, -radius },
            { -radius, height, -radius },
            { -radius, height, +radius },
            { +radius, height, +radius }
        };
        scene.Models.back().Mesh.TexCoords = {
            { +15, -15 },
            { -15, -15 },
            { -15, +15 },
            { +15, +15 }
        };
        scene.Models.back().Mesh.Indices = { 0, 1, 2, 0, 2, 3 };
        scene.Materials.emplace_back();
        scene.Materials.back().Albedo = Engine::LoadImageRGBA("assets/images/ground.jpg");
        scene.Materials.back().MetaSpec.Fill(glm::vec4(0));
        scene.Materials.back().Height.Fill(0);
    }

    static void AddGround(Engine::Scene & scene, float height) {
        float const radius = scene.Cameras[0].ZFar * .5f;
        scene.Models.emplace_back();
        scene.Models.back().MaterialIndex = std::uint32_t(scene.Materials.size());
        scene.Models.back().Mesh.Positions = {
            { +radius, height, -radius },
            { -radius, height, -radius },
            { -radius, height, +radius },
            { +radius, height, +radius }
        };
        scene.Models.back().Mesh.TexCoords = {
            { +15, -15 },
            { -15, -15 },
            { -15, +15 },
            { +15, +15 }
        };
        scene.Models.back().Mesh.Indices = { 0, 1, 2, 0, 2, 3 };
        scene.Materials.emplace_back();
        scene.Materials.back().Albedo = Engine::LoadImageRGBA("assets/images/ground.jpg");
        scene.Materials.back().MetaSpec.Fill(glm::vec4(0));
        scene.Materials.back().Height.Fill(0);
    }

    static std::array<Engine::Scene, Assets::ExampleScenes.size()> LoadExampleScenes() {
        std::array<Engine::Scene, Assets::ExampleScenes.size()> scenes;
        for (std::size_t i = 0; i < scenes.size(); i++) {
            scenes[i] = Engine::LoadScene(Assets::ExampleScenes[i]);
            if (i == std::size_t(Assets::ExampleScene::SportsCar)) {
                AddGround(scenes[i]);
            } else if (i == std::size_t(Assets::ExampleScene::WhiteOak)) {
                AddGround(scenes[i], -10.);
            }
        }
        return scenes;
    }

    static std::array<std::string, Assets::ExampleScenes.size()> LoadExampleSceneNames() {
        std::array<std::string, Assets::ExampleScenes.size()> names;
        for (std::size_t i = 0; i < names.size(); i++) {
            names[i] = std::filesystem::path(Assets::ExampleScenes[i]).stem().filename().string();
        }
        return names;
    }

    std::array<Engine::Scene, Assets::ExampleScenes.size()> const Content::Scenes = LoadExampleScenes();
    std::array<std::string,   Assets::ExampleScenes.size()> const Content::SceneNames  = LoadExampleSceneNames();
} // namespace VCX::Labs::Rendering
