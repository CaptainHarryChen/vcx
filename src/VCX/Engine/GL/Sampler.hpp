#pragma once

#include "Engine/GL/resource.hpp"

namespace VCX::Engine::GL {
    struct SamplerTrait {
        static auto constexpr & CreateMany = glGenSamplers;
        static auto constexpr & DeleteMany = glDeleteSamplers;
    };
    
    enum class WrapMode : GLenum {
        Clamp  = GL_CLAMP_TO_EDGE,
        Repeat = GL_REPEAT,
    };

    enum class FilterMode : GLenum {
        Nearest   = GL_NEAREST,
        Linear    = GL_LINEAR,
        Bilinear  = GL_LINEAR_MIPMAP_NEAREST,
        Trilinear = GL_LINEAR_MIPMAP_LINEAR,
    };

    struct SamplerOptions {
        WrapMode   WrapU     = WrapMode::Clamp;
        WrapMode   WrapV     = WrapMode::Clamp;
        FilterMode MinFilter;
        FilterMode MagFilter;
    };

    class UniqueSampler : public Unique<SamplerTrait> {
    public:
        UniqueSampler() = default;
        
        explicit UniqueSampler(SamplerOptions && options) {
            glSamplerParameteri(Get(), GL_TEXTURE_WRAP_S, GLenum(options.WrapU));
            glSamplerParameteri(Get(), GL_TEXTURE_WRAP_T, GLenum(options.WrapV));
            glSamplerParameteri(Get(), GL_TEXTURE_MIN_FILTER, GLenum(options.MinFilter));
            glSamplerParameteri(Get(), GL_TEXTURE_MAG_FILTER, GLenum(options.MagFilter));
        }
    };
}
