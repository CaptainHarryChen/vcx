#pragma once

#include <optional>
#include <vector>

#include "Engine/GL/resource.hpp"
#include "Engine/prelude.hpp"
#include "Engine/type.hpp"

using GLenum = unsigned int;
using GLuint = unsigned int;

namespace VCX::Engine::GL {
    template<typename T>
    inline constexpr GLenum TypeEnumOf = 0;
    template<>
    inline constexpr GLenum TypeEnumOf<float> = GL_FLOAT;

    struct VertexAttrib {
        GLuint Location;
        GLuint Offset;
        GLenum Type;
        GLuint Size;
        bool   Normalized;
    };

    struct VertexLayout {
        std::size_t                 Stride;
        std::vector<VertexAttrib> Attributes;

        template<typename T>
        VertexLayout Of() && {
            Stride = sizeof(T);
            return std::move(*(this));
        }

        template<typename T, typename TField>
        VertexLayout At(
            GLuint const location,
            TField T::* const field,
            bool const        normalized = false) && {
            Attributes.push_back(VertexAttrib {
                .Location { location },
                .Offset { GLuint(std::intptr_t(&((*(T *) 0).*field))) },
                .Type { TypeEnumOf<glm_type_of<TField>> },
                .Size { glm_size_of_v<TField> },
                .Normalized { normalized },
            });
            return std::move(*(this));
        }
    };

    class UniqueMesh {
    public:
        UniqueMesh(
            VertexLayout const &               layout,
            std::span<std::byte const> const & data,
            std::span<GLuint const> const &    elements,
            GLenum                             mode = GL_TRIANGLES);

        UniqueMesh(
            VertexLayout const &               layout,
            std::span<std::byte const> const & data,
            GLenum                             mode = GL_TRIANGLES);

        void Draw(std::initializer_list<scope_t> && scopes);

    private:
        GLenum                                  _mode;
        std::size_t                             _count;
        UniqueVertexArray                       _vao;
        UniqueArrayBuffer                       _vbo;
        std::optional<UniqueElementArrayBuffer> _ebo;
    };
} // namespace VCX::Engine::GL
