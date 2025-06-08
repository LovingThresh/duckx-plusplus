#pragma once

namespace duckx
{
    typedef unsigned const int formatting_flag;

    // text-formatting flags
    constexpr formatting_flag none = 0;
    constexpr formatting_flag bold = 1 << 0;
    constexpr formatting_flag italic = 1 << 1;
    constexpr formatting_flag underline = 1 << 2;
    constexpr formatting_flag strikethrough = 1 << 3;
    constexpr formatting_flag superscript = 1 << 4;
    constexpr formatting_flag subscript = 1 << 5;
    constexpr formatting_flag smallcaps = 1 << 6;
    constexpr formatting_flag shadow = 1 << 7;

    enum class Alignment {
        LEFT,
        CENTER,
        RIGHT,
        BOTH
    };
} // namespace duckx
