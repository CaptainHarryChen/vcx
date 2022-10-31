#include <filesystem>
#include <fstream>

#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <tiny_obj_loader.h>

#include "Engine/loader.h"

namespace std {
    template<>
    struct hash<tuple<int, int, int>> {
        size_t operator()(tuple<int, int, int> const & val) const {
            auto [i1, i2, i3] = val;
            return ((i1 ^ (i2 << 1)) >> 1) ^ (i3 << 1);
        }
    };
}

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

    Texture2D<Formats::R8G8B8A8> LoadImageRGBA(std::filesystem::path const & fileName, bool const flipped) {
        auto const buf { LoadBytes(fileName) };
        int        width {}, height {}, channels {};
        stbi_set_flip_vertically_on_load(flipped);
        auto const image {
            stbi_load_from_memory(
                reinterpret_cast<const stbi_uc *>(buf.data()),
                buf.size(),
                &width,
                &height,
                &channels,
                4)
        };
        Texture2D<Formats::R8G8B8A8> texture(width, height);
        std::memcpy(
            reinterpret_cast<void *>(const_cast<std::byte *>(texture.GetBytes().data())),
            image,
            texture.GetBytes().size());
        stbi_image_free(image);
        return texture;
    }

    static SurfaceMesh LoadSurfaceMeshOBJ(std::filesystem::path const & fileName, bool const simplified) {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;

        std::ifstream file(fileName);
        bool const    ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &file);

        if (! warn.empty())
            spdlog::warn("VCX::Engine::LoadSurfaceMeshOBJ(\"{}\"): {}", fileName.filename().string(), warn);
        if (! err.empty())
            spdlog::error("VCX::Engine::LoadSurfaceMeshOBJ(\"{}\"): {}", fileName.filename().string(), err);
        if (! ret)
            return {};

        spdlog::trace("VCX::Engine::LoadSurfaceMeshOBJ(\"{}\")", fileName.filename().string());

        SurfaceMesh                                                  mesh;
        std::unordered_map<std::tuple<int, int, int>, std::uint32_t> uniqueVertices;

        for (auto const & shape : shapes) {
            for (auto const & index : shape.mesh.indices) {
                auto const vertex = simplified
                    ? std::tuple(index.vertex_index, -1, -1)
                    : std::tuple(index.vertex_index, index.normal_index, index.texcoord_index);
                if (auto const iter = uniqueVertices.find(vertex); iter != uniqueVertices.end()) {
                    mesh.Indices.push_back(iter->second);
                } else {
                    mesh.Indices.push_back(uniqueVertices[vertex] = std::uint32_t(mesh.Positions.size()));

                    mesh.Positions.emplace_back(attrib.vertices[index.vertex_index * 3 + 0], attrib.vertices[index.vertex_index * 3 + 1], attrib.vertices[index.vertex_index * 3 + 2]);
                    if (index.normal_index >= 0)
                        mesh.Normals.emplace_back(attrib.normals[index.normal_index * 3 + 0], attrib.normals[index.normal_index * 3 + 1], attrib.normals[index.normal_index * 3 + 2]);
                    if (index.texcoord_index >= 0)
                        mesh.TexCoords.emplace_back(attrib.texcoords[index.texcoord_index * 2 + 0], 1 - attrib.texcoords[index.texcoord_index * 2 + 1]);
                }
            }
        }
        return mesh;
    }

    SurfaceMesh LoadSurfaceMesh(std::filesystem::path const & fileName, bool const simplified) {
        auto const ext = fileName.extension();
        if (ext == ".obj") {
            return LoadSurfaceMeshOBJ(fileName, simplified);
        } else {
            spdlog::error("VCX::Engine::LoadSurfaceMesh(\"{}\"): undertermined file format.", fileName.filename().string());
            return {};
        }
    }
} // namespace VCX::Engine
