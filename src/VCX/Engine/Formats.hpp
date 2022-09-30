#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>

#include <glm/glm.hpp>

namespace VCX::Engine {
    // clang-format off
    template<typename T> concept TextureFormat =
        std::is_same_v<decltype(T::Decode(std::declval<typename T::Encoded const &>())), typename T::Decoded> &&
        std::is_same_v<decltype(T::Encode(std::declval<typename T::Decoded const &>())), typename T::Encoded>;
    // clang-format on

    namespace Formats {
        struct R8G8B8 {
            using Decoded = glm::vec3;
            using Encoded = glm::u8vec3;

            static Decoded Decode(Encoded const & o) {
                return Decoded(o) * _dec;
            }

            static Encoded Encode(Decoded const & o) {
                return {
                    std::round(std::clamp<float>(o.r, 0, 1) * _enc),
                    std::round(std::clamp<float>(o.g, 0, 1) * _enc),
                    std::round(std::clamp<float>(o.b, 0, 1) * _enc),
                };
            }

        private:
            static constexpr float _enc { 255. };
            static constexpr float _dec { 1. / 255. };
        };

        struct R8G8B8A8 {
            using Decoded = glm::vec4;
            using Encoded = glm::u8vec4;

            static Decoded Decode(Encoded const & o) {
                return Decoded(o) * _dec;
            }

            static Encoded Encode(Decoded const & o) {
                return {
                    std::round(std::clamp<float>(o.r, 0, 1) * _enc),
                    std::round(std::clamp<float>(o.g, 0, 1) * _enc),
                    std::round(std::clamp<float>(o.b, 0, 1) * _enc),
                    std::round(std::clamp<float>(o.a, 0, 1) * _enc),
                };
            }

            template<TextureFormat NewFormat>
            static typename NewFormat::Encoded Cast(const Encoded &val) {
                static_assert(std::is_same_v<NewFormat, R8G8B8>, "invalid cast");
                if constexpr (std::is_same_v<NewFormat, R8G8B8>)
                    return { val.r, val.g, val.b };
            }

        private:
            static constexpr float _enc { 255. };
            static constexpr float _dec { 1. / 255. };
        };
    } // namespace Formats

} // namespace VCX::Engine
