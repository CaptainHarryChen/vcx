#pragma once

#include <optional>

#include <spdlog/spdlog.h>

#include "Engine/GL/resource.hpp"
#include "Engine/GL/Texture2D.hpp"
#include "Engine/prelude.hpp"

namespace VCX::Engine::GL {
    enum class FrameType {
        ColorOnly, ColorAndDepth, ColorAndStencil, ColorAndDepthStencil
    };

    template<TextureFormat Format = Formats::R8G8B8A8, std::size_t NumColorAttachments = 1, FrameType Type = FrameType::ColorAndDepthStencil>
    class UniqueFrame {
    public:
        explicit UniqueFrame(SamplerOptions && options = { .MinFilter = FilterMode::Linear, .MagFilter = FilterMode::Nearest }) :
            _colorAttachments(make_array<UniqueTexture2D, NumColorAttachments>(std::forward<SamplerOptions>(options))) {
            gl_using(_fbo);

            for (std::size_t i = 0; i < NumColorAttachments; ++i) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _colorAttachments[i].Get(), 0);
            }
        }

        void Resize(std::pair<std::uint32_t, std::uint32_t> const size) {
            if (_size == size) return;
            auto const [width, height] = size;
            _size = size;

            for (std::size_t i = 0; i < NumColorAttachments; ++i) {
                _colorAttachments[i].template Resize<Format>(width, height);
            }

            gl_using(_fbo);

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
                spdlog::error("VCX::Engine::GL::UniqueFrame::Resize(..): incomplete framebuffer.");
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

        template<std::size_t Idx = 0> UniqueTexture2D const & GetColorAttachment() const {
            _colorAttachments[Idx].GenerateMipmap();
            return _colorAttachments[Idx];
        }

        std::pair<std::uint32_t, std::uint32_t> GetSize() const { return _size; }

    private:
        std::array<UniqueTexture2D, NumColorAttachments> _colorAttachments;
        UniqueFramebuffer _fbo;
        UniqueRenderbuffer _rbo;

        std::pair<std::uint32_t, std::uint32_t> _size = { 0, 0 };
    };
}
