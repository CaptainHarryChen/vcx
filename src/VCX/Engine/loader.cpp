#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>
#include <stb_image.h>

#include "Engine/loader.h"

namespace VCX::Engine {
    std::vector<std::byte> LoadBytes(std::filesystem::path const & fileName) {
        if (std::filesystem::exists(fileName)) {
            std::ifstream file(fileName, std::ios::binary);
            file.seekg(0, std::ios::end);
            const auto fileSize { static_cast<std::size_t>(file.tellg()) };
            file.seekg(0, std::ios::beg);
            std::vector<std::byte> blob;
            blob.resize(fileSize);
            file.read(reinterpret_cast<char *>(blob.data()), blob.size());
            file.close();
            spdlog::trace("VCX::Engine::LoadBytes(\"{}\")", fileName.filename().string());
            return blob;
        } else {
            spdlog::error("VCX::Engine::LoadBytes(\"{}\"): not found.", fileName.filename().string());
            return {};
        }
    }

    Texture2D<Formats::R8G8B8A8> LoadImageRGBA(std::filesystem::path const & path) {
        auto const buf { LoadBytes(path) };
        int        width {}, height {}, channels {};
        auto const image {
            stbi_load_from_memory(
                reinterpret_cast<const stbi_uc *>(buf.data()),
                buf.size(),
                &width,
                &height,
                &channels,
                4)
        };
        Texture2D<Formats::R8G8B8A8> texture({ std::size_t(width), std::size_t(height) });
        std::memcpy(
            reinterpret_cast<void *>(const_cast<std::byte *>(texture.GetBytes().data())),
            image,
            texture.GetBytes().size());
        stbi_image_free(image);
        return texture;
    }
} // namespace VCX::Engine
