/*
 * @file: BaseElement.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "BaseElement.hpp"

#include <cctype>
#include <iostream>
#include <zip.h>

namespace duckx
{
    DocxElement::DocxElement(const pugi::xml_node parentNode, const pugi::xml_node currentNode)
        : m_parentNode(parentNode), m_currentNode(currentNode)
    {
    }

    void Run::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:r");
    }

    void Run::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    pugi::xml_node DocxElement::findNextSibling(const std::string& name) const
    {
        const pugi::xml_node sibling = m_currentNode.next_sibling(name.c_str());
        return sibling;
    }

    Run::Run(const pugi::xml_node parent, const pugi::xml_node current) : DocxElement(parent, current)
    {
    }

    std::string Run::get_text() const
    {
        return m_currentNode.child("w:t").text().get();
    }

    bool Run::set_text(const std::string& text) const
    {
        return m_currentNode.child("w:t").text().set(text.c_str());
    }

    bool Run::set_text(const char* text) const
    {
        return m_currentNode.child("w:t").text().set(text);
    }

    Run& Run::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        return *this;
    }

    bool Run::has_next() const
    {
        return m_currentNode != nullptr;
    }

    TableCell::TableCell(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current)
    {
    }

    void TableCell::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tc");

        m_paragraph.set_parent(m_currentNode);
    }

    void TableCell::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    bool TableCell::has_next() const
    {
        return m_currentNode != nullptr;
    }

    TableCell& TableCell::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        return *this;
    }

    ElementRange<Paragraph> TableCell::paragraphs()
    {
        if (m_currentNode)
        {
            m_paragraph.set_current(m_currentNode.child("w:p"));
        }
        else
        {
            m_paragraph.set_current(pugi::xml_node());
        }

        m_paragraph.set_parent(m_currentNode);

        return ElementRange<Paragraph>(m_paragraph);
    }

    TableRow::TableRow(const pugi::xml_node parent, const pugi::xml_node current) : DocxElement(parent, current)
    {
    }

    void TableRow::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tr");

        m_tableCell.set_parent(m_currentNode);
    }

    void TableRow::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    TableRow& TableRow::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        return *this;
    }

    ElementRange<TableCell> TableRow::cells()
    {
        if (m_currentNode)
        {
            m_tableCell.set_current(m_currentNode.child("w:tc"));
        }
        else
        {
            m_tableCell.set_current(pugi::xml_node());
        }

        m_tableCell.set_parent(m_currentNode);

        return ElementRange<TableCell>(m_tableCell);
    }

    bool TableRow::has_next() const
    {
        return m_currentNode != nullptr;
    }

    Table::Table(const pugi::xml_node parent, const pugi::xml_node current) : DocxElement(parent, current)
    {
    }

    void Table::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tbl");

        m_tableRow.set_parent(m_currentNode);
    }

    bool Table::has_next() const
    {
        return m_currentNode != nullptr;
    }

    Table& Table::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        m_tableRow.set_parent(m_currentNode);
        return *this;
    }

    void Table::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    ElementRange<TableRow> Table::rows()
    {
        if (m_currentNode)
        {
            m_tableRow.set_current(m_currentNode.child("w:tr"));
        }
        else
        {
            m_tableRow.set_current(pugi::xml_node());
        }

        m_tableRow.set_parent(m_currentNode);

        return ElementRange<TableRow>(m_tableRow);
    }

    Paragraph::Paragraph(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current)
    {
    }

    void Paragraph::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:p");

        m_run.set_parent(m_currentNode);
    }

    void Paragraph::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    Paragraph& Paragraph::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        m_run.set_parent(m_currentNode);
        return *this;
    }

    bool Paragraph::has_next() const
    {
        return m_currentNode != nullptr;
    }

    ElementRange<Run> Paragraph::runs()
    {
        if (m_currentNode)
        {
            m_run.set_current(m_currentNode.child("w:r"));
        }
        else
        {
            m_run.set_current(pugi::xml_node());
        }

        m_run.set_parent(m_currentNode);

        return ElementRange<Run>(m_run);
    }

    Run& Paragraph::add_run(const std::string& text, formatting_flag f)
    {
        return add_run(text.c_str(), f);
    }

    Run& Paragraph::add_run(const char* text, formatting_flag f)
    {
        // Add new run
        pugi::xml_node new_run = m_currentNode.append_child("w:r");
        // Insert meta to new run
        pugi::xml_node meta = new_run.append_child("w:rPr");

        if (f & bold)
            meta.append_child("w:b");

        if (f & italic)
            meta.append_child("w:i");

        if (f & underline)
            meta.append_child("w:u").append_attribute("w:val").set_value("single");

        if (f & strikethrough)
            meta.append_child("w:strike").append_attribute("w:val").set_value("true");

        if (f & superscript)
            meta.append_child("w:vertAlign").append_attribute("w:val").set_value("superscript");
        else if (f & subscript)
            meta.append_child("w:vertAlign").append_attribute("w:val").set_value("subscript");

        if (f & smallcaps)
            meta.append_child("w:smallCaps").append_attribute("w:val").set_value("true");

        if (f & shadow)
            meta.append_child("w:shadow").append_attribute("w:val").set_value("true");

        pugi::xml_node new_run_text = new_run.append_child("w:t");

        if (*text != 0 && (isspace(text[0]) || isspace(text[strlen(text) - 1])))
            new_run_text.append_attribute("xml:space").set_value("preserve");
        new_run_text.text().set(text);

        return *new Run(m_currentNode, new_run);
    }

    Paragraph& Paragraph::insert_paragraph_after(const std::string& text, formatting_flag f)
    {
        const pugi::xml_node new_para = m_parentNode.insert_child_after("w:p", m_currentNode);

        const auto p = new Paragraph();
        p->set_current(new_para);
        p->add_run(text, f);

        return *p;
    }
} // namespace duckx
