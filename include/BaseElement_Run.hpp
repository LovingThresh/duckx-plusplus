/*!
 * @file BaseElement_Run.hpp
 * @brief Text run element implementation
 * 
 * Contains the Run class for handling text runs within paragraphs,
 * including formatting, style application, and text manipulation.
 * 
 * @date 2025.07
 */
#pragma once

#include "BaseElement_Core.hpp"

namespace duckx
{
    class StyleManager;

    /*!
     * @brief Text run element within a paragraph
     * 
     * Represents a contiguous piece of text with consistent formatting.
     * Runs are the atomic units of text formatting in DOCX documents.
     */
    class DUCKX_API Run : public DocxElement
    {
    public:
        Run() = default;
        Run(pugi::xml_node, pugi::xml_node);
        Run(const Run&) = default;
        Run& operator=(const Run&) = default;
        Run(Run&&) = default;
        Run& operator=(Run&&) = default;
        bool has_next() const override;
        bool has_next_same_type() const override;
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;

        /*! @brief Set the text content of this run */
        bool set_text(const std::string&) const;
        /*! @brief Set the text content of this run */
        bool set_text(const char*) const;
        /*! @brief Set the font family name */
        Run& set_font(const std::string& font_name);
        /*! @brief Set the font size in points */
        Run& set_font_size(double size);
        /*! @brief Set the text color (hex format like "FF0000") */
        Run& set_color(const std::string& color);
        /*! @brief Set the highlight color */
        Run& set_highlight(HighlightColor color);
        /*! @brief Get the text content of this run */
        std::string get_text() const;

        formatting_flag get_formatting() const;
        bool is_bold() const;
        bool is_italic() const;
        bool is_underline() const;
        bool get_font(std::string& font_name) const;
        bool get_font_size(double& size) const;
        bool get_color(std::string& color) const;
        bool get_highlight(HighlightColor& color) const;

        Run& advance();
        bool try_advance();
        bool can_advance() const;
        bool move_to_next_run();

        // Modern Result<T> API for style application (recommended)
        /*! @brief Safely apply a character style by name */
        Result<void> apply_style_safe(const StyleManager& style_manager, const std::string& style_name);
        /*! @brief Safely get the currently applied style name */
        Result<std::string> get_style_safe() const;
        /*! @brief Safely remove style from this run */
        Result<void> remove_style_safe();

    private:
        pugi::xml_node get_or_create_rPr();
    };

} // namespace duckx