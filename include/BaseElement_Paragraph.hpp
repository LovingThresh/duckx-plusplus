/*!
 * @file BaseElement_Paragraph.hpp
 * @brief Paragraph element implementation
 * 
 * Contains the Paragraph class for handling document paragraphs,
 * including text runs, formatting, alignment, and style management.
 * 
 * @date 2025.07
 */
#pragma once

#include "BaseElement_Core.hpp"
#include "BaseElement_Run.hpp"
#include <array>

namespace duckx
{
    class Document;
    class StyleManager;

    /*!
     * @brief Paragraph element containing text runs and formatting
     * 
     * Represents a paragraph in the document, which can contain multiple
     * text runs with different formatting, hyperlinks, and paragraph-level
     * properties like alignment and spacing.
     */
    class DUCKX_API Paragraph : public DocxElement
    {
    public:
        Paragraph() = default;
        Paragraph(pugi::xml_node, pugi::xml_node);
        Paragraph(const Paragraph&) = default;
        Paragraph& operator=(const Paragraph&) = default;
        Paragraph(Paragraph&&) = default;
        Paragraph& operator=(Paragraph&&) = default;
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;
        bool has_next_same_type() const override;

        absl::enable_if_t<is_docx_element<Run>::value, ElementRange<Run>> runs();
        absl::enable_if_t<is_docx_element<Run>::value, ElementRange<Run>> runs() const;
        /*! @brief Add a text run with optional formatting */
        Run& add_run(const std::string&, duckx::formatting_flag = duckx::none);
        /*! @brief Add a text run with optional formatting */
        Run& add_run(const char*, duckx::formatting_flag = duckx::none);
        /*! @brief Add a hyperlink run to the paragraph */
        Run add_hyperlink(const Document& doc, const std::string& text, const std::string& url);
        /*! @brief Set paragraph text alignment */
        Paragraph& set_alignment(Alignment align);
        /*! @brief Set paragraph spacing before and after */
        Paragraph& set_spacing(double before_pts = -1, double after_pts = -1);
        /*! @brief Set line spacing within the paragraph */
        Paragraph& set_line_spacing(double line_spacing);
        /*! @brief Set left and right indentation */
        Paragraph& set_indentation(double left_pts = -1, double right_pts = -1);
        /*! @brief Set first line indentation */
        Paragraph& set_first_line_indent(double first_line_pts);
        /*! @brief Apply list formatting to the paragraph */
        Paragraph& set_list_style(ListType type, int level = 0);
        Paragraph& insert_paragraph_after(const std::string& = "", duckx::formatting_flag = duckx::none);
        Paragraph& advance();
        bool try_advance();
        bool can_advance() const;
        bool move_to_next_paragraph();

        Alignment get_alignment() const;
        bool get_line_spacing(double& line_spacing) const;
        bool get_spacing(double& before_pts, double& after_pts) const;
        bool get_indentation(double& left_pts, double& right_pts, double& first_line_pts) const;
        bool get_list_style(ListType& type, int& level, int& numId) const;

        // Modern Result<T> API for style application (recommended)
        /*! @brief Safely apply a paragraph style by name */
        Result<void> apply_style_safe(const StyleManager& style_manager, const std::string& style_name);
        /*! @brief Safely get the currently applied style name */
        Result<std::string> get_style_safe() const;
        /*! @brief Safely remove style from this paragraph */
        Result<void> remove_style_safe();

    private:
        pugi::xml_node get_or_create_pPr();

        Run m_run;
    };

} // namespace duckx