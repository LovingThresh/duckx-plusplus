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

#include "duckxiterator.hpp"
#include "duckx_export.h"

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

        pugi::xml_node getNode() const;
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
        pugi::xml_node findNextSibling(const std::string& name) const;

        pugi::xml_node findNextAnySibling() const;
        static ElementType mapStringToElementType(const std::string& node_name);
        static ElementType determineElementType(pugi::xml_node node);

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

    private:
        Paragraph m_paragraph;
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

    private:
        TableCell m_tableCell;
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

    private:
        TableRow m_tableRow;
    };
} // namespace duckx
