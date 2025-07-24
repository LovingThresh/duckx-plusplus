/*
 * @file: BaseElement.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
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

    class DUCKX_API DocxElement
    {
    public:
        DocxElement() = default;
        DocxElement(pugi::xml_node parentNode, pugi::xml_node currentNode);

        virtual ~DocxElement() = default;
        virtual bool has_next() const = 0;
        virtual bool has_next_same_type() const = 0;
        virtual void set_parent(pugi::xml_node) = 0;
        virtual void set_current(pugi::xml_node) = 0;

        pugi::xml_node get_node() const;
        bool has_next_sibling() const;

        enum class ElementType
        {
            PARAGRAPH,
            TABLE,
            RUN,
            TABLE_ROW,
            TABLE_CELL,
            UNKNOWN
        };

        struct SiblingInfo
        {
            ElementType type;
            std::string tag_name;
            bool exists;

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

    // Run contains runs in a paragraph
    class DUCKX_API Run : public DocxElement
    {
    public:
        Run() = default;
        Run(pugi::xml_node, pugi::xml_node);
        bool has_next() const override;
        bool has_next_same_type() const override;
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;

        bool set_text(const std::string&) const;
        bool set_text(const char*) const;
        Run& set_font(const std::string& font_name);
        Run& set_font_size(double size);
        Run& set_color(const std::string& color);
        Run& set_highlight(HighlightColor color);
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

    private:
        pugi::xml_node get_or_create_rPr();
    };

    class DUCKX_API Paragraph : public DocxElement
    {
    public:
        Paragraph() = default;
        Paragraph(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;
        bool has_next_same_type() const override;

        absl::enable_if_t<is_docx_element<Run>::value, ElementRange<Run>> runs();
        absl::enable_if_t<is_docx_element<Run>::value, ElementRange<Run>> runs() const;
        Run& add_run(const std::string&, duckx::formatting_flag = duckx::none);
        Run& add_run(const char*, duckx::formatting_flag = duckx::none);
        Run add_hyperlink(const Document& doc, const std::string& text, const std::string& url);
        Paragraph& set_alignment(Alignment align);
        Paragraph& set_spacing(double before_pts = -1, double after_pts = -1);
        Paragraph& set_line_spacing(double line_spacing);
        Paragraph& set_indentation(double left_pts = -1, double right_pts = -1);
        Paragraph& set_first_line_indent(double first_line_pts);
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

    private:
        pugi::xml_node get_or_create_pPr();

        Run m_run;
    };

    // TableCell contains one or more paragraphs
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

        // Add paragraph to cell
        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);

        // Modern Result<T> API for formatting (recommended)
        Result<TableCell*> set_width_safe(double width_pts);
        Result<TableCell*> set_width_type_safe(const std::string& type);
        Result<TableCell*> set_vertical_alignment_safe(const std::string& alignment);
        Result<TableCell*> set_background_color_safe(const std::string& color);
        Result<TableCell*> set_text_direction_safe(const std::string& direction);
        Result<TableCell*> set_margins_safe(double top_pts, double right_pts, double bottom_pts, double left_pts);
        Result<TableCell*> set_border_style_safe(const std::string& style);
        Result<TableCell*> set_border_width_safe(double width_pts);
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

    // TableRow consists of one or more TableCells
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
        Result<TableRow*> set_height_safe(double height_pts);
        Result<TableRow*> set_height_rule_safe(const std::string& rule);
        Result<TableRow*> set_header_row_safe(bool is_header);
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

        TableCell& add_cell();
        TableCell& get_cell(int index);
        int cell_count() const;

    private:
        TableCell m_tableCell;
        
        // Helper methods
        pugi::xml_node get_or_create_tr_pr();
    };

    // Table consists of one or more TableRow objects
    class DUCKX_API Table : public DocxElement
    {
    public:
        Table() = default;
        Table(pugi::xml_node, pugi::xml_node);
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
        Result<Table*> set_alignment_safe(const std::string& alignment);
        Result<Table*> set_width_safe(double width_pts);
        Result<Table*> set_border_style_safe(const std::string& style);
        Result<Table*> set_border_width_safe(double width_pts);
        Result<Table*> set_border_color_safe(const std::string& color);
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

        TableRow& add_row();
        TableRow& get_row(int index);
        int row_count() const;

    private:
        TableRow m_tableRow;
    };

} // namespace duckx
