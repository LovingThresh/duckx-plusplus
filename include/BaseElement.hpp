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

#include "duckxiterator.hpp"

namespace duckx
{
    class DocxElement
    {
    public:
        DocxElement() = default;
        DocxElement(pugi::xml_node parentNode, pugi::xml_node currentNode);

        virtual ~DocxElement() = default;
        virtual bool has_next() const = 0;
        virtual void set_parent(pugi::xml_node) = 0;
        virtual void set_current(pugi::xml_node) = 0;
        pugi::xml_node getNode() const;

    protected:
        pugi::xml_node findNextSibling(const std::string& name) const;

        pugi::xml_node m_parentNode; // Parent node in the XML tree
        pugi::xml_node m_currentNode; // Current node in the XML tree
    };

    // Run contains runs in a paragraph
    class Run : public DocxElement
    {
    public:
        Run() = default;
        Run(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;

        bool set_text(const std::string&) const;
        bool set_text(const char*) const;
        Run& set_font(const std::string& font_name);
        Run& set_font_size(double size);
        Run& set_color(const std::string& color);
        Run& set_highlight(HighlightColor color);
        std::string get_text() const;
        Run& next();

    private:
        pugi::xml_node get_or_create_rPr();
    };

    // Paragraph contains a paragraph
    // and stores runs
    class Paragraph : public DocxElement
    {
    public:
        Paragraph() = default;
        Paragraph(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;

        ElementRange<Run> runs();
        Run& add_run(const std::string&, duckx::formatting_flag = duckx::none);
        Run& add_run(const char*, duckx::formatting_flag = duckx::none);
        Paragraph& set_alignment(Alignment align);
        Paragraph& set_spacing(double before_pts = -1, double after_pts = -1);
        Paragraph& set_line_spacing(double line_spacing);
        Paragraph& set_indentation(double left_pts = -1, double right_pts = -1);
        Paragraph& set_first_line_indent(double first_line_pts);
        Paragraph& set_list_style(ListType type, int level = 0);
        Paragraph& insert_paragraph_after(const std::string& = "", duckx::formatting_flag = duckx::none);
        Paragraph& next();

    private:
        pugi::xml_node get_or_create_pPr();

        Run m_run;
    };

    // TableCell contains one or more paragraphs
    class TableCell : public DocxElement
    {
    public:
        TableCell() = default;
        TableCell(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;

        ElementRange<Paragraph> paragraphs();
        TableCell& next();

    private:
        Paragraph m_paragraph;
    };

    // TableRow consists of one or more TableCells
    class TableRow : public DocxElement
    {
    public:
        TableRow() = default;
        TableRow(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;

        ElementRange<TableCell> cells();
        TableRow& next();

    private:
        TableCell m_tableCell;
    };

    // Table consists of one or more TableRow objects
    class Table : public DocxElement
    {
    public:
        Table() = default;
        Table(pugi::xml_node, pugi::xml_node);
        void set_parent(pugi::xml_node) override;
        void set_current(pugi::xml_node) override;
        bool has_next() const override;

        Table& next();
        ElementRange<TableRow> rows();

    private:
        TableRow m_tableRow;
    };
} // namespace duckx
