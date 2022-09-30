#pragma once

#include <filesystem>
#include <initializer_list>
#include <memory>
#include <span>

#include "Engine/GL/resource.hpp"
#include "Engine/prelude.hpp"

namespace VCX::Engine::GL {
    enum class ShaderType : GLenum {
        None,
        Compute  = GL_COMPUTE_SHADER,
        Vertex   = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
    };

    static ShaderType ShaderTypeFromExtension(std::filesystem::path const & ext) {
        if (ext == ".comp") return ShaderType::Compute;
        if (ext == ".vert") return ShaderType::Vertex;
        if (ext == ".frag") return ShaderType::Fragment;
        if (ext == ".geom") return ShaderType::Geometry;
        return ShaderType::None;
    }

    struct ShaderTrait {
        static auto constexpr & Delete = glDeleteShader;
    };

    class SharedShader : public Shared<ShaderTrait> {
    public:
        // clang-format off
        SharedShader(            std::filesystem::path  const &);
        SharedShader(ShaderType, std::filesystem::path  const &);
        SharedShader(ShaderType, std::vector<std::byte> const &);
        // clang-format on
    };

    struct ProgramTrait {
        static auto constexpr & Create = glCreateProgram;
        static auto constexpr & Delete = glDeleteProgram;
        static auto constexpr & Bind   = glUseProgram;
    };

    class UniqueProgram : public Unique<ProgramTrait> {
    public:
        UniqueProgram(std::initializer_list<SharedShader> &&);
    };
} // namespace VCX::Engine::GL
