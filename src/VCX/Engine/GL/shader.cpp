#include <array>

#include <spdlog/spdlog.h>

#include "Engine/GL/shader.h"
#include "Engine/loader.h"

namespace VCX::Engine::GL {
    static void CheckShader(GLuint);
    static void CheckProgram(GLuint);

    SharedShader::SharedShader(std::filesystem::path const & fileName):
        SharedShader(ShaderTypeFromExtension(fileName.extension()), fileName) {}

    SharedShader::SharedShader(
        ShaderType const            type,
        std::filesystem::path const & fileName):
        SharedShader(type, LoadBytes(fileName)) {}

    SharedShader::SharedShader(
        ShaderType const             type,
        std::vector<std::byte> const & blob):
        Shared(glCreateShader(static_cast<GLenum>(type))) {
        if (type == ShaderType::None) {
            spdlog::error("VCX::Engine::GL::SharedShader(..): undetermined shader type.");
            std::exit(EXIT_FAILURE);
        }

        std::array<GLchar const *, 1> sources { reinterpret_cast<GLchar const *>(blob.data()) };
        std::array<GLint, 1>          lengths { static_cast<GLint>(blob.size()) };
        auto const                    shader { Get() };
        glShaderSource(shader, 1, sources.data(), lengths.data());
        glCompileShader(shader);
        CheckShader(shader);
    }

    UniqueProgram::UniqueProgram(std::initializer_list<SharedShader> && shaders) {
        auto const program { Get() };
        for (auto const & shader : shaders) {
            glAttachShader(program, shader.Get());
        }
        glLinkProgram(program);
        CheckProgram(program);
    }

    static void CheckShader(GLuint const shader) {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success) {
            spdlog::trace("glCompileShader({})", shader);
        } else {
            spdlog::error("glCompileShader({}): failed.", shader);
            std::array<GLchar, 1024> buf;
            glGetShaderInfoLog(shader, buf.size(), nullptr, buf.data());
            spdlog::error("\t{}", buf.data());
            std::exit(EXIT_FAILURE);
        }
    }

    static void CheckProgram(GLuint const program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success) {
            spdlog::trace("glLinkProgram({})", program);
        } else {
            spdlog::error("glLinkProgram({}): failed.", program);
            std::array<GLchar, 1024> buf;
            glGetProgramInfoLog(program, buf.size(), nullptr, buf.data());
            spdlog::error("\t{}", buf.data());
            std::exit(EXIT_FAILURE);
        }
    }
} // namespace VCX::Engine::GL
