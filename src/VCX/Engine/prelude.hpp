#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

namespace VCX::Engine {
    template<typename T>
    std::span<std::byte const>
        make_span_bytes(std::span<T const> const & span) {
        return std::span<std::byte const>(
            (std::byte const *) span.data(),
            span.size_bytes());
    }
}
