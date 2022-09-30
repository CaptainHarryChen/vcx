#pragma once

#include <optional>

#include <spdlog/spdlog.h>

#include "Engine/GL/resource.hpp"

namespace VCX::Engine::GL {
    enum class FrameType {
        ColorOnly, ColorAndDepth, ColorAndStencil, ColorAndDepthStencil
    };

    template<std::size_t NumColorAttachments = 1, FrameType Type = FrameType::ColorAndDepthStencil>
    class UniqueFrame {
    public:
        UniqueFrame() = default;
        UniqueFrame(std::pair<std::uint32_t, std::uint32_t> const size) { Resize(size); }

        void Resize(std::pair<std::uint32_t, std::uint32_t> const size) {
            if (_size == size) return;
            auto const [width, height] = size;
            _size = size;

            gl_using(_fbo);
            
            for (std::size_t i = 0; i < NumColorAttachments; ++i) {
                auto & attachment { _colorAttachments[i] };
                gl_using(attachment);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, attachment.Get(), 0);
            }

            if constexpr (Type != FrameType::ColorOnly) {
                gl_using(_rbo);
                if constexpr (Type == FrameType::ColorAndDepthStencil) {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo.Get());
                } else if constexpr (Type == FrameType::ColorAndDepth) {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbo.Get());
                } else {
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo.Get());
                }
            }

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                spdlog::error("VCX::Engine::GL::Frame::Resize(..): incomplete framebuffer.");
                std::exit(EXIT_FAILURE);
            }
        }
        
        scope_t Use() const {
            auto ret = _fbo.Use();
            glViewport(0, 0, _size.first, _size.second);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            return ret;
        }

        template<std::size_t Idx = 0> UniqueTexture2D const & GetColorAttachment() const { return _colorAttachments[Idx]; }
        std::pair<std::uint32_t, std::uint32_t> GetSize() const { return _size; }

    private:
        std::array<UniqueTexture2D, NumColorAttachments> _colorAttachments;
        UniqueFramebuffer _fbo;
        UniqueRenderbuffer _rbo;

        std::pair<std::uint32_t, std::uint32_t> _size = { 0, 0 };
    };
}
