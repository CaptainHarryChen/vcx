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
}
