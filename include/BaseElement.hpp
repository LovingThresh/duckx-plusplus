/*!
 * @file BaseElement.hpp
 * @brief Base element classes for DOCX document structure
 * 
 * Defines the core element hierarchy including paragraphs, runs, tables,
 * rows, and cells with both modern Result<T> and legacy exception-based APIs.
 * Provides the foundation for all document content manipulation with
 * comprehensive formatting capabilities.
 * 
 * @date 2025.07
 */
#pragma once

#include <constants.hpp>
#include <pugixml.hpp>
#include <string>
#include <utility>
#include <array>

#include "duckxiterator.hpp"
#include "duckx_export.h"
#include "Error.hpp"

namespace duckx
{
    class Document;
    class StyleManager;

    /*!
     * @brief Abstract base class for all DOCX document elements
     * 
     * Provides common navigation and XML node management functionality
     * for all document elements like paragraphs, tables, runs, etc.
     */
    class DUCKX_API DocxElement
    {
    public:
        DocxElement() = default;
        DocxElement(pugi::xml_node parentNode, pugi::xml_node currentNode);
        DocxElement(const DocxElement&) = default;
        DocxElement& operator=(const DocxElement&) = default;
        DocxElement(DocxElement&&) = default;
        DocxElement& operator=(DocxElement&&) = default;

        virtual ~DocxElement() = default;
        /*! @brief Check if there's a next element of any type */
        virtual bool has_next() const = 0;
        /*! @brief Check if there's a next element of the same type */
        virtual bool has_next_same_type() const = 0;
        /*! @brief Set the parent XML node */
        virtual void set_parent(pugi::xml_node) = 0;
        /*! @brief Set the current XML node */
        virtual void set_current(pugi::xml_node) = 0;

        pugi::xml_node get_node() const;
        bool has_next_sibling() const;

        /*! @brief Enumeration of document element types */
        enum class ElementType
        {
            PARAGRAPH,   //!< Text paragraph element
            TABLE,       //!< Table element
            RUN,         //!< Text run within paragraph
            TABLE_ROW,   //!< Table row element
            TABLE_CELL,  //!< Table cell element
            UNKNOWN      //!< Unknown or unsupported element type
        };

        /*! @brief Information about sibling elements */
        struct SiblingInfo
        {
            ElementType type;     //!< Type of the sibling element
            std::string tag_name; //!< XML tag name of the sibling
            bool exists;          //!< Whether the sibling exists

            SiblingInfo()
                : type(ElementType::UNKNOWN), exists(false) {}

            SiblingInfo(const ElementType t, std::string tag)
                : type(t), tag_name(std::move(tag)), exists(true) {}
        };

        SiblingInfo peek_next_sibling() const;

    protected:
        pugi::xml_node find_next_sibling(const std::string& name) const;

        pugi::xml_node find_next_any_sibling() const;
        static ElementType map_string_to_element_type(const std::string& node_name);
        static ElementType determine_element_type(pugi::xml_node node);

        pugi::xml_node m_parentNode; // Parent node in the XML tree
        pugi::xml_node m_currentNode; // Current node in the XML tree
    };

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

    /*!
     * @brief Table cell element containing paragraphs
     * 
     * Represents a single cell in a table that can contain paragraphs
     * and has formatting properties like width, alignment, borders, and colors.
     * Supports both modern Result<T> and legacy APIs.
     */
    class DUCKX_API TableCell : public DocxElement
    {
    public:
        TableCell() = default;
        TableCell(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;
        bool has_next_same_type() const override;

        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs();
        TableCell& advance();
        bool try_advance();
        bool can_advance() const;
        bool move_to_next_cell();

        /*! @brief Add a paragraph to this cell */
        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);

        // Modern Result<T> API for formatting (recommended)
        /*! @brief Safely set cell width in points */
        Result<TableCell*> set_width_safe(double width_pts);
        /*! @brief Safely set cell width type ("auto", "pct", "dxa") */
        Result<TableCell*> set_width_type_safe(const std::string& type);
        /*! @brief Safely set vertical alignment ("top", "center", "bottom") */
        Result<TableCell*> set_vertical_alignment_safe(const std::string& alignment);
        /*! @brief Safely set background color (hex format) */
        Result<TableCell*> set_background_color_safe(const std::string& color);
        /*! @brief Safely set text direction ("ltr", "rtl") */
        Result<TableCell*> set_text_direction_safe(const std::string& direction);
        /*! @brief Safely set cell margins in points */
        Result<TableCell*> set_margins_safe(double top_pts, double right_pts, double bottom_pts, double left_pts);
        /*! @brief Safely set border style ("single", "double", etc.) */
        Result<TableCell*> set_border_style_safe(const std::string& style);
        /*! @brief Safely set border width in points */
        Result<TableCell*> set_border_width_safe(double width_pts);
        /*! @brief Safely set border color (hex format) */
        Result<TableCell*> set_border_color_safe(const std::string& color);

        // Legacy API (for backward compatibility)
        TableCell& set_width(double width_pts);
        TableCell& set_width_type(const std::string& type);
        TableCell& set_vertical_alignment(const std::string& alignment);
        TableCell& set_background_color(const std::string& color);
        TableCell& set_text_direction(const std::string& direction);
        TableCell& set_margins(double top_pts, double right_pts, double bottom_pts, double left_pts);
        TableCell& set_border_style(const std::string& style);
        TableCell& set_border_width(double width_pts);
        TableCell& set_border_color(const std::string& color);

        // Getters
        double get_width() const;
        std::string get_width_type() const;
        std::string get_vertical_alignment() const;
        std::string get_background_color() const;
        std::string get_text_direction() const;
        Result<std::array<double, 4>> get_margins_safe() const;
        std::string get_border_style() const;
        double get_border_width() const;
        std::string get_border_color() const;

    private:
        Paragraph m_paragraph;
        
        // Helper methods
        pugi::xml_node get_or_create_tc_pr();
        pugi::xml_node get_or_create_tc_borders(pugi::xml_node tc_pr);
    };

    /*!
     * @brief Table row element containing cells
     * 
     * Represents a row in a table containing one or more cells.
     * Provides formatting options like height, header row status,
     * and row-level properties.
     */
    class DUCKX_API TableRow : public DocxElement
    {
    public:
        TableRow() = default;
        TableRow(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;
        bool has_next_same_type() const override;

        absl::enable_if_t<is_docx_element<TableCell>::value, ElementRange<TableCell>> cells();
        TableRow& advance();
        bool try_advance();
        bool can_advance() const;
        bool move_to_next_row();

        // Modern Result<T> API for formatting (recommended)
        /*! @brief Safely set row height in points */
        Result<TableRow*> set_height_safe(double height_pts);
        /*! @brief Safely set height rule ("auto", "exact", "atLeast") */
        Result<TableRow*> set_height_rule_safe(const std::string& rule);
        /*! @brief Safely set whether this is a header row */
        Result<TableRow*> set_header_row_safe(bool is_header);
        /*! @brief Safely set whether the row can be split across pages */
        Result<TableRow*> set_cant_split_safe(bool cant_split);

        // Legacy API (for backward compatibility)
        TableRow& set_height(double height_pts);
        TableRow& set_height_rule(const std::string& rule);
        TableRow& set_header_row(bool is_header);
        TableRow& set_cant_split(bool cant_split);

        // Getters
        double get_height() const;
        std::string get_height_rule() const;
        bool is_header_row() const;
        bool get_cant_split() const;

        /*! @brief Add a new cell to this row */
        TableCell& add_cell();
        /*! @brief Get cell at specified index */
        TableCell& get_cell(int index);
        /*! @brief Get the number of cells in this row */
        int cell_count() const;

    private:
        TableCell m_tableCell;
        
        // Helper methods
        pugi::xml_node get_or_create_tr_pr();
    };

    /*!
     * @brief Table element containing rows and cells
     * 
     * Represents a table in the document with formatting capabilities
     * including alignment, borders, cell margins, and overall table styling.
     * Supports both modern Result<T> and legacy APIs.
     */
    class DUCKX_API Table : public DocxElement
    {
    public:
        Table() = default;
        Table(pugi::xml_node, pugi::xml_node);
        Table(const Table&) = default;
        Table& operator=(const Table&) = default;
        Table(Table&&) = default;
        Table& operator=(Table&&) = default;
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;
        bool has_next_same_type() const override;

        absl::enable_if_t<is_docx_element<TableRow>::value, ElementRange<TableRow>> rows();
        Table& advance();
        bool try_advance();
        bool can_advance() const;
        bool move_to_next_table();

        // Modern Result<T> API for formatting (recommended)
        /*! @brief Safely set table alignment ("left", "center", "right") */
        Result<Table*> set_alignment_safe(const std::string& alignment);
        /*! @brief Safely set table width in points */
        Result<Table*> set_width_safe(double width_pts);
        /*! @brief Safely set border style for all borders */
        Result<Table*> set_border_style_safe(const std::string& style);
        /*! @brief Safely set border width for all borders */
        Result<Table*> set_border_width_safe(double width_pts);
        /*! @brief Safely set border color for all borders */
        Result<Table*> set_border_color_safe(const std::string& color);
        /*! @brief Safely set default cell margins for all cells */
        Result<Table*> set_cell_margins_safe(double top_pts, double right_pts, double bottom_pts, double left_pts);

        // Legacy API (for backward compatibility)
        Table& set_alignment(const std::string& alignment);
        Table& set_width(double width_pts);
        Table& set_border_style(const std::string& style);
        Table& set_border_width(double width_pts);
        Table& set_border_color(const std::string& color);
        Table& set_cell_margins(double top_pts, double right_pts, double bottom_pts, double left_pts);

        // Getters with Result<T> for complex operations
        std::string get_alignment() const;
        double get_width() const;
        std::string get_border_style() const;
        double get_border_width() const;
        std::string get_border_color() const;
        Result<std::array<double, 4>> get_cell_margins_safe() const;

        /*! @brief Add a new row to this table */
        TableRow& add_row();
        /*! @brief Get row at specified index */
        TableRow& get_row(int index);
        /*! @brief Get the number of rows in this table */
        int row_count() const;

        // Modern Result<T> API for style application (recommended)
        /*! @brief Safely apply a table style by name */
        Result<void> apply_style_safe(const StyleManager& style_manager, const std::string& style_name);
        /*! @brief Safely get the currently applied style name */
        Result<std::string> get_style_safe() const;
        /*! @brief Safely remove style from this table */
        Result<void> remove_style_safe();

    private:
        TableRow m_tableRow;
    };

} // namespace duckx
