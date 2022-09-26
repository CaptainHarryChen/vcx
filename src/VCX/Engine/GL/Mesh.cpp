#include "Engine/GL/Mesh.h"

#include <spdlog/spdlog.h>

namespace VCX::Engine::GL {
    UniqueMesh::UniqueMesh(
        VertexLayout const &               layout,
        std::span<std::byte const> const & data,
        std::span<GLuint const> const &    elements,
        GLenum const                       mode) :
        _mode(mode),
        _count(elements.size()),
        _ebo(UniqueElementArrayBuffer()) {
        _vao.Bind();
        _vbo.Bind();
        _ebo.value().Bind();
        glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size_bytes(), elements.data(), GL_STATIC_DRAW);
        for (auto const & attr : layout.Attributes) {
            glVertexAttribPointer(
                attr.Location, attr.Size, attr.Type, attr.Normalized, layout.Stride,
                reinterpret_cast<void *>(std::uintptr_t(attr.Offset)));
            glEnableVertexAttribArray(attr.Location);
        }
        _vao.Unbind();
        _vbo.Unbind();
        _ebo.value().Unbind();
    }

    UniqueMesh::UniqueMesh(
        VertexLayout const &               layout,
        std::span<std::byte const> const & data,
        GLenum                             mode) :
        _mode(mode),
        _count(data.size() / layout.Stride) {
        _vao.Bind();
        _vbo.Bind();
        glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
        for (auto const & attr : layout.Attributes) {
            glVertexAttribPointer(
                attr.Location, attr.Size, attr.Type, attr.Normalized, layout.Stride,
                reinterpret_cast<void *>(std::uintptr_t(attr.Offset)));
            glEnableVertexAttribArray(attr.Location);
        }
        _vao.Unbind();
        _vbo.Unbind();
    }

    void UniqueMesh::Draw(std::initializer_list<scope_t> && scopes) {
        gl_using(_vao);
        if (_ebo.has_value())
            glDrawElements(_mode, _count, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(_mode, 0, _count);
    }
}
