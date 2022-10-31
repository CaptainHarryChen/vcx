#pragma once

#include <Engine/prelude.hpp>
#include <Engine/SurfaceMesh.h>
#include <Engine/TextureND.hpp>

namespace VCX::Engine {
    // load all bytes of a binary file into a dynamically-allocated memory.
    // If the file does not exist, this function returns an empty vector,
    // and an error will be emitted to spdlog.
    std::vector<std::byte> LoadBytes(std::filesystem::path const & fileName);

    Texture2D<Formats::R8G8B8A8> LoadImageRGBA(std::filesystem::path const & fileName, bool const flipped = false);

    SurfaceMesh LoadSurfaceMesh(std::filesystem::path const & fileName, bool const simplified = false);
}
