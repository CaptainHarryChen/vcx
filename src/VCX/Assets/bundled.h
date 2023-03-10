#pragma once

#include <array>
#include <string_view>

namespace VCX::Assets {
    inline constexpr auto DefaultIcons {
        std::to_array<std::string_view>({
            "assets/images/vcl-logo-32x32.png",
            "assets/images/vcl-logo-48x48.png",
        })
    };

    inline constexpr auto DefaultFonts {
        std::to_array<std::string_view>({
            "assets/fonts/Ubuntu.ttf",
            "assets/fonts/UbuntuMono.ttf",
        })
    };

    inline constexpr auto ExampleModels {
        std::to_array<std::string_view>({
            "assets/models/arma.obj",
            "assets/models/block.obj",
            "assets/models/cube.obj",
            "assets/models/dinosaur.obj",
            "assets/models/face.obj",
            "assets/models/fandisk.obj",
            "assets/models/rocker.obj",
            "assets/models/sphere.obj",
        })
    };

    enum class ExampleModel {
        Arma,
        Block,
        Cube,
        Dinosaur,
        Face,
        Fandisk,
        Rocker,
        Sphere,
    };

    inline constexpr auto ExampleScenes {
        std::to_array<std::string_view>({
            "assets/scenes/floor/floor.yaml",
            "assets/scenes/cornell_box/cornell_box.yaml",
            "assets/scenes/cornell_box_sphere/cornell_box_sphere.yaml",
            "assets/scenes/cornell_box_mirror/CornellBox-mirror.yaml",
            "assets/scenes/cornell_box_water/cornellbox_water2.yaml",
            "assets/scenes/triangular_prism/triangular_prism.yaml",
            "assets/scenes/teapot/teapot.yaml",
            "assets/scenes/bunny/bunny.yaml",
            "assets/scenes/sponza/sponza.yaml",
            "assets/scenes/breakfast_room/breakfast_room.yaml",
            "assets/scenes/white_oak/white_oak.yaml",
            "assets/scenes/sports_car/sports_car.yaml",
            "assets/scenes/sibenik/sibenik.yaml",
        })
    };

    enum class ExampleScene {
        Floor,
        CornellBox,
        CornellBoxSphere,
        CornellBoxMirror,
        CornellBoxWater,
        TriangularPrism,
        Teapot,
        Bunny,
        Sponza,
        BreakfastRoom,
        WhiteOak,
        SportsCar,
        Sibenik,
    };
}
