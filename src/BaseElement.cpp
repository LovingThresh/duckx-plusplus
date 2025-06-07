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

duckx::DocxElement::DocxElement(const pugi::xml_node parentNode, const pugi::xml_node currentNode)
    : m_parentNode(parentNode), m_currentNode(currentNode)
{
}

void duckx::Run::set_parent(const pugi::xml_node node)
{
    this->m_parentNode = node;
    this->m_currentNode = this->m_parentNode.child("w:r");
}

void duckx::Run::set_current(const pugi::xml_node node)
{
    this->m_currentNode = node;
}

pugi::xml_node duckx::DocxElement::findNextSibling(const std::string& name) const
{
    const pugi::xml_node sibling = m_currentNode.next_sibling(name.c_str());
    return sibling;
}

duckx::Run::Run(const pugi::xml_node parent, const pugi::xml_node current) : DocxElement(parent, current)
{
}

std::string duckx::Run::get_text() const
{
    return this->m_currentNode.child("w:t").text().get();
}

bool duckx::Run::set_text(const std::string& text) const
{
    return this->m_currentNode.child("w:t").text().set(text.c_str());
}

bool duckx::Run::set_text(const char* text) const
{
    return this->m_currentNode.child("w:t").text().set(text);
}

duckx::Run& duckx::Run::next()
{
    this->m_currentNode = this->m_currentNode.next_sibling();
    return *this;
}

bool duckx::Run::has_next() const
{
    return this->m_currentNode != nullptr;
}

duckx::TableCell::TableCell(const pugi::xml_node parent, const pugi::xml_node current)
    : DocxElement(parent, current)
{
}

void duckx::TableCell::set_parent(const pugi::xml_node node)
{
    this->m_parentNode = node;
    this->m_currentNode = this->m_parentNode.child("w:tc");

    this->m_paragraph.set_parent(this->m_currentNode);
}

void duckx::TableCell::set_current(const pugi::xml_node node)
{
    this->m_currentNode = node;
}

bool duckx::TableCell::has_next() const
{
    return this->m_currentNode != nullptr;
}

duckx::TableCell& duckx::TableCell::next()
{
    this->m_currentNode = this->m_currentNode.next_sibling();
    return *this;
}

duckx::Paragraph& duckx::TableCell::paragraphs()
{
    this->m_paragraph.set_parent(this->m_currentNode);
    return this->m_paragraph;
}

duckx::TableRow::TableRow(const pugi::xml_node parent, const pugi::xml_node current)
    : DocxElement(parent, current)
{
}

void duckx::TableRow::set_parent(const pugi::xml_node node)
{
    this->m_parentNode = node;
    this->m_currentNode = this->m_parentNode.child("w:tr");

    this->m_tableCell.set_parent(this->m_currentNode);
}

void duckx::TableRow::set_current(const pugi::xml_node node)
{
    this->m_currentNode = node;
}

duckx::TableRow& duckx::TableRow::next()
{
    this->m_currentNode = this->m_currentNode.next_sibling();
    return *this;
}

duckx::TableCell& duckx::TableRow::cells()
{
    this->m_tableCell.set_parent(this->m_currentNode);
    return this->m_tableCell;
}

bool duckx::TableRow::has_next() const
{
    return this->m_currentNode != nullptr;
}

duckx::Table::Table(const pugi::xml_node parent, const pugi::xml_node current)
    : DocxElement(parent, current)
{
}

void duckx::Table::set_parent(const pugi::xml_node node)
{
    this->m_parentNode = node;
    this->m_currentNode = this->m_parentNode.child("w:tbl");

    this->m_tableRow.set_parent(this->m_currentNode);
}

bool duckx::Table::has_next() const
{
    return this->m_currentNode != nullptr;
}

duckx::Table& duckx::Table::next()
{
    this->m_currentNode = this->m_currentNode.next_sibling();
    this->m_tableRow.set_parent(this->m_currentNode);
    return *this;
}

void duckx::Table::set_current(const pugi::xml_node node)
{
    this->m_currentNode = node;
}

duckx::TableRow& duckx::Table::rows()
{
    this->m_tableRow.set_parent(this->m_currentNode);
    return this->m_tableRow;
}

duckx::Paragraph::Paragraph(const pugi::xml_node parent, const pugi::xml_node current) : DocxElement(parent, current)
{
}

void duckx::Paragraph::set_parent(const pugi::xml_node node)
{
    this->m_parentNode = node;
    this->m_currentNode = this->m_parentNode.child("w:p");

    this->m_run.set_parent(this->m_currentNode);
}

void duckx::Paragraph::set_current(const pugi::xml_node node)
{
    this->m_currentNode = node;
}

duckx::Paragraph& duckx::Paragraph::next()
{
    this->m_currentNode = this->m_currentNode.next_sibling();
    this->m_run.set_parent(this->m_currentNode);
    return *this;
}

bool duckx::Paragraph::has_next() const
{
    return this->m_currentNode != nullptr;
}

duckx::Run& duckx::Paragraph::runs()
{
    this->m_run.set_parent(this->m_currentNode);
    return this->m_run;
}

duckx::Run& duckx::Paragraph::add_run(const std::string& text, duckx::formatting_flag f)
{
    return this->add_run(text.c_str(), f);
}

duckx::Run& duckx::Paragraph::add_run(const char* text, duckx::formatting_flag f)
{
    // Add new run
    pugi::xml_node new_run = this->m_currentNode.append_child("w:r");
    // Insert meta to new run
    pugi::xml_node meta = new_run.append_child("w:rPr");

    if (f & duckx::bold)
        meta.append_child("w:b");

    if (f & duckx::italic)
        meta.append_child("w:i");

    if (f & duckx::underline)
        meta.append_child("w:u").append_attribute("w:val").set_value("single");

    if (f & duckx::strikethrough)
        meta.append_child("w:strike").append_attribute("w:val").set_value("true");

    if (f & duckx::superscript)
        meta.append_child("w:vertAlign").append_attribute("w:val").set_value("superscript");
    else if (f & duckx::subscript)
        meta.append_child("w:vertAlign").append_attribute("w:val").set_value("subscript");

    if (f & duckx::smallcaps)
        meta.append_child("w:smallCaps").append_attribute("w:val").set_value("true");

    if (f & duckx::shadow)
        meta.append_child("w:shadow").append_attribute("w:val").set_value("true");

    pugi::xml_node new_run_text = new_run.append_child("w:t");
    // If the run starts or ends with whitespace characters, preserve them using
    // the xml:space attribute
    if (*text != 0 && (isspace(text[0]) || isspace(text[strlen(text) - 1])))
        new_run_text.append_attribute("xml:space").set_value("preserve");
    new_run_text.text().set(text);

    return *new Run(this->m_currentNode, new_run);
}

duckx::Paragraph& duckx::Paragraph::insert_paragraph_after(const std::string& text, duckx::formatting_flag f)
{
    const pugi::xml_node new_para = this->m_parentNode.insert_child_after("w:p", this->m_currentNode);

    const auto p = new Paragraph();
    p->set_current(new_para);
    p->add_run(text, f);

    return *p;
}



