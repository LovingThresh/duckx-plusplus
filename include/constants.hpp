/*!
 * @file constants.hpp
 * @brief Common constants and enumerations for DOCX formatting
 * 
 * Defines formatting flags, alignment options, list types, and other
 * constants used throughout the DuckX library. Provides type-safe
 * enumerations and bitwise flags for comprehensive document formatting.
 * 
 * @date 2025.07
 */
#pragma once

#include "duckx_export.h"

namespace duckx
{
    /*! @brief Type for text formatting flag combinations */
    typedef unsigned int formatting_flag;
    /*! @brief Default preset ID for formatting */
    constexpr int preset_id = 4;

    // Text formatting flags (can be combined with bitwise OR)
    constexpr formatting_flag none = 0;                  //!< No formatting
    constexpr formatting_flag bold = 1 << 0;             //!< Bold text
    constexpr formatting_flag italic = 1 << 1;           //!< Italic text
    constexpr formatting_flag underline = 1 << 2;        //!< Underlined text
    constexpr formatting_flag strikethrough = 1 << 3;    //!< Strikethrough text
    constexpr formatting_flag superscript = 1 << 4;      //!< Superscript text
    constexpr formatting_flag subscript = 1 << 5;        //!< Subscript text
    constexpr formatting_flag smallcaps = 1 << 6;        //!< Small capitals
    constexpr formatting_flag shadow = 1 << 7;           //!< Shadow effect

    /*! @brief Text alignment options for paragraphs */
    enum class DUCKX_API Alignment
    {
        LEFT,   //!< Left aligned text
        CENTER, //!< Center aligned text
        RIGHT,  //!< Right aligned text
        BOTH    //!< Justified text (both left and right aligned)
    };

    /*! @brief List formatting types for paragraphs */
    enum class DUCKX_API ListType
    {
        NONE,   //!< Not a list item
        BULLET, //!< Bullet point list
        NUMBER  //!< Numbered list
    };

    /*! @brief Text highlight color options */
    enum class DUCKX_API HighlightColor
    {
        NONE,         //!< No highlight
        BLACK,        //!< Black highlight
        BLUE,         //!< Blue highlight
        CYAN,         //!< Cyan highlight
        GREEN,        //!< Green highlight
        MAGENTA,      //!< Magenta highlight
        RED,          //!< Red highlight
        YELLOW,       //!< Yellow highlight (default)
        WHITE,        //!< White highlight
        DARK_BLUE,    //!< Dark blue highlight
        DARK_CYAN,    //!< Dark cyan highlight
        DARK_GREEN,   //!< Dark green highlight
        DARK_MAGENTA, //!< Dark magenta highlight
        DARK_RED,     //!< Dark red highlight
        DARK_YELLOW,  //!< Dark yellow highlight
        LIGHT_GRAY    //!< Light gray highlight
        // MS Word supports 16 standard highlight colors
    };

    /*! @brief Header and footer type specifications */
    enum class DUCKX_API HeaderFooterType
    {
        DEFAULT, //!< Default header/footer for all pages
        FIRST,   //!< Header/footer for first page only
        EVEN,    //!< Header/footer for even pages
        ODD      //!< Header/footer for odd pages
    };
} // namespace duckx
