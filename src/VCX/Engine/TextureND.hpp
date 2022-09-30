#pragma once

#include <array>
#include <vector>

#include "Engine/Formats.hpp"
#include "Engine/prelude.hpp"

namespace VCX::Engine {
    template<std::size_t Dim, TextureFormat Format>
    class TextureND {
        static_assert(Dim == 1 || Dim == 2 || Dim == 3);

        template<std::size_t OtherDim, TextureFormat OtherFormat>
        friend class TextureND;

    private:
        std::array<std::size_t, Dim>          _size;
        std::vector<typename Format::Encoded> _data;

        /** Throws std::out_of_range if a invalid index is given */
        constexpr std::size_t GetIndexAt(std::array<std::size_t, Dim> const & o) const {
            // clang-format off
            if constexpr (Dim >= 1) if (o[0] >= _size[0]) throw std::out_of_range("x is out of range.");
            if constexpr (Dim >= 2) if (o[1] >= _size[1]) throw std::out_of_range("y is out of range.");
            if constexpr (Dim >= 3) if (o[2] >= _size[2]) throw std::out_of_range("z is out of range.");
            // clang-format on

            if constexpr (Dim == 1) return o[0];
            if constexpr (Dim == 2) return o[1] * _size[0] + o[0];
            if constexpr (Dim == 3) return (o[2] * _size[1] + o[1]) * _size[0] + o[0];
        }

    public:
        explicit TextureND(std::array<std::size_t, Dim> const & o):
            _size(o) {
            if constexpr (Dim == 1) _data.resize(o[0]);
            if constexpr (Dim == 2) _data.resize(o[0] * o[1]);
            if constexpr (Dim == 3) _data.resize(o[0] * o[2]);
        }

        template<TextureFormat NewFormat>
            requires requires (typename Format::Encoded a) { { Format::template Cast<NewFormat>(a) } -> std::same_as<typename NewFormat::Encoded>; }
        TextureND<Dim, NewFormat> Cast() {
            TextureND<Dim, NewFormat> result(_size);
            auto sourceIter = _data.begin();
            auto resultIter = result._data.begin();
            while (sourceIter != _data.end()) {
                *resultIter = Format::template Cast<NewFormat>(*sourceIter);
                ++ sourceIter;
                ++ resultIter;
            }
            return result;
        }

        std::span<std::byte const> GetBytes() const {
            return make_span_bytes<typename Format::Encoded>(_data);
        }

        std::array<std::size_t, Dim> GetSize() const { return _size; }

        // clang-format off
        template<std::size_t D = Dim> requires (Dim >= 1) std::size_t GetSizeX() const { return _size[0]; }
        template<std::size_t D = Dim> requires (Dim >= 2) std::size_t GetSizeY() const { return _size[1]; }
        template<std::size_t D = Dim> requires (Dim >= 3) std::size_t GetSizeZ() const { return _size[2]; }
        // clang-format on

        typename Format::Decoded operator[](std::array<std::size_t, Dim> const & o) const {
            return GetAt(o);
        }

        typename Format::Decoded GetAt(std::array<std::size_t, Dim> const & o) const {
            return Format::Decode(_data[GetIndexAt(o)]);
        }

        void SetAt(
            std::array<std::size_t, Dim> const & o,
            typename Format::Decoded const &              oo) {
            _data[GetIndexAt(o)] = Format::Encode(oo);
        }

        void Fill(typename Format::Decoded const & o) {
            auto const oo { Format::Encode(o) };
            for (auto & o : _data) o = oo;
        }
    };

    // clang-format off
    template<TextureFormat Format> using Texture1D = TextureND<1, Format>;
    template<TextureFormat Format> using Texture2D = TextureND<2, Format>;
    template<TextureFormat Format> using Texture3D = TextureND<3, Format>;
    // clang-format on
} // namespace VCX::Engine
