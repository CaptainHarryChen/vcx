#pragma once

#include <functional>
#include <memory>

#include <glad/glad.h>

#include "scope.hpp"

#define gl_using(var) \
    auto const __use_##var { (var).Use() }

namespace VCX::Engine::GL {
    using scope_t = scope::scope_exit<void (&)()>;

    // clang-format off
    template<typename T> concept ResourceCreate =
        std::is_same_v<decltype(T::Create()), GLuint>;
    template<typename T> concept ResourceCreateMany =
        std::is_same_v<decltype(T::CreateMany(
            std::declval<GLsizei >(),
            std::declval<GLuint *>()
        )), void>;
    template<typename T> concept ResourceDelete =
        std::is_same_v<decltype(T::Delete(
            std::declval<GLuint>()
        )), void>;
    template<typename T> concept ResourceDeleteMany =
        std::is_same_v<decltype(T::DeleteMany(
            std::declval<GLsizei       >(),
            std::declval<GLuint const *>()
        )), void>;
    template<typename T> concept ResourceBind =
        std::is_same_v<decltype(T::Bind(
            std::declval<GLuint>()
        )), void>;
    template<typename T> concept ResourceBindToTarget =
        std::is_same_v<std::remove_const_t<decltype(T::BindTarget)>, GLenum> &&
        std::is_same_v<decltype(T::Bind(
            std::declval<GLenum>(),
            std::declval<GLuint>()
        )), void>;

    template<typename T> concept ResourceTrait =
        // (ResourceCreate<T> || ResourceCreateMany<T>) &&
        (ResourceDelete<T> || ResourceDeleteMany<T>);// &&
        // (ResourceBind  <T> || ResourceBindToTarget<T>);
    // clang-format on

    template<ResourceTrait Trait>
    struct ResourceMethods {
        static GLuint Create() {
            if constexpr (ResourceCreate<Trait>)
                return Trait::Create();
            if constexpr (ResourceCreateMany<Trait>) {
                GLuint o { 0 };
                Trait::CreateMany(1, &o);
                return o;
            }
        }

        static void Delete(GLuint const o) {
            if constexpr (ResourceDelete<Trait>)
                return Trait::Delete(o);
            if constexpr (ResourceDeleteMany<Trait>)
                return Trait::DeleteMany(1, &o);
        }

        static void Bind(GLuint const o) {
            if constexpr (ResourceBind<Trait>)
                Trait::Bind(o);
            if constexpr (ResourceBindToTarget<Trait>)
                Trait::Bind(Trait::BindTarget, o);
        }

        static void Unbind() {
            if constexpr (ResourceBind<Trait>)
                Trait::Bind(0);
            if constexpr (ResourceBindToTarget<Trait>)
                Trait::Bind(Trait::BindTarget, 0);
        }
    };

    template<ResourceTrait Trait>
    class Unique {
    private:
        scope::unique_resource<GLuint, void (*)(GLuint)> _o;

    public:
        template<typename T = Trait>
            requires(ResourceCreate<Trait> || ResourceCreateMany<Trait>)
        Unique():
            _o(scope::make_unique_resource(
                ResourceMethods<Trait>::Create(),
                ResourceMethods<Trait>::Delete)) {}
        explicit Unique(GLuint const o):
            _o(scope::make_unique_resource(
                o,
                ResourceMethods<Trait>::Delete)) {}
        GLuint Get() const { return _o.get(); }
        template<typename T = Trait>
            requires(ResourceBind<Trait> || ResourceBindToTarget<Trait>)
        scope_t Use()
        const {
            ResourceMethods<Trait>::Bind(_o.get());
            return scope::make_scope_exit(
                ResourceMethods<Trait>::Unbind);
        }
        template<typename T = Trait>
            requires(ResourceBind<Trait> || ResourceBindToTarget<Trait>)
        void Bind() const { ResourceMethods<Trait>::Bind(_o.get()); }
        template<typename T = Trait>
            requires(ResourceBind<Trait> || ResourceBindToTarget<Trait>)
        void Unbind() const { ResourceMethods<Trait>::Unbind(); }
    };

    template<ResourceTrait Trait>
    class Shared {
    private:
        std::shared_ptr<scope::unique_resource<GLuint, void (*)(GLuint)>> _o;

    public:
        template<typename T = Trait>
            requires(ResourceCreate<Trait> || ResourceCreateMany<Trait>)
        Shared():
            _o(std::make_shared<scope::unique_resource<GLuint, void (*)(GLuint)>>(scope::make_unique_resource(
                ResourceMethods<Trait>::Create(),
                ResourceMethods<Trait>::Delete))) {}
        explicit Shared(GLuint const o):
            _o(std::make_shared<scope::unique_resource<GLuint, void (*)(GLuint)>>(scope::make_unique_resource(
                o,
                ResourceMethods<Trait>::Delete))) {}
        GLuint Get() const { return _o->get(); }
        template<typename T = Trait>
            requires(ResourceBind<Trait> || ResourceBindToTarget<Trait>)
        scope_t Use()
        const {
            ResourceMethods<Trait>::Bind(_o.get());
            return scope::make_scope_exit(
                ResourceMethods<Trait>::Unbind);
        }
        template<typename T = Trait>
            requires(ResourceBind<Trait> || ResourceBindToTarget<Trait>)
        void Bind() const { ResourceMethods<Trait>::Bind(_o.get()); }
        template<typename T = Trait>
            requires(ResourceBind<Trait> || ResourceBindToTarget<Trait>)
        void Unbind() const { ResourceMethods<Trait>::Unbind(); }
    };

    struct UniqueVertexArrayTrait {
        static auto constexpr & CreateMany = glGenVertexArrays;
        static auto constexpr & DeleteMany = glDeleteVertexArrays;
        static auto constexpr & Bind       = glBindVertexArray;
    };

    struct UniqueArrayBufferTrait {
        static auto constexpr & CreateMany = glGenBuffers;
        static auto constexpr & DeleteMany = glDeleteBuffers;
        static auto constexpr & Bind       = glBindBuffer;
        static GLenum constexpr BindTarget = GL_ARRAY_BUFFER;
    };

    struct UniqueElementArrayBufferTrait {
        static auto constexpr & CreateMany = glGenBuffers;
        static auto constexpr & DeleteMany = glDeleteBuffers;
        static auto constexpr & Bind       = glBindBuffer;
        static GLenum constexpr BindTarget = GL_ELEMENT_ARRAY_BUFFER;
    };

    struct UniqueTexture2DTrait {
        static auto constexpr & CreateMany = glGenTextures;
        static auto constexpr & DeleteMany = glDeleteTextures;
        static auto constexpr & Bind       = glBindTexture;
        static GLenum constexpr BindTarget = GL_TEXTURE_2D;
    };

    struct UniqueFramebufferTrait {
        static auto constexpr & CreateMany = glGenFramebuffers;
        static auto constexpr & DeleteMany = glDeleteFramebuffers;
        static auto constexpr & Bind       = glBindFramebuffer;
        static GLenum constexpr BindTarget = GL_FRAMEBUFFER;
    };

    struct UniqueRenderbufferTrait {
        static auto constexpr & CreateMany = glGenRenderbuffers;
        static auto constexpr & DeleteMany = glDeleteRenderbuffers;
        static auto constexpr & Bind       = glBindRenderbuffer;
        static GLenum constexpr BindTarget = GL_RENDERBUFFER;
    };

    // clang-format on

    using UniqueVertexArray        = Unique<UniqueVertexArrayTrait>;
    using UniqueArrayBuffer        = Unique<UniqueArrayBufferTrait>;
    using UniqueElementArrayBuffer = Unique<UniqueElementArrayBufferTrait>;
    using UniqueTexture2D          = Unique<UniqueTexture2DTrait>;
    using UniqueFramebuffer        = Unique<UniqueFramebufferTrait>;
    using UniqueRenderbuffer       = Unique<UniqueRenderbufferTrait>;
} // namespace VCX::Engine::GL
