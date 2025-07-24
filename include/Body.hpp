/*
 * @file: Body.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#pragma once
#include "BaseElement.hpp"
#include "duckx_export.h"

namespace duckx
{
    class DUCKX_API Body
    {
    public:
        Body() = default;
        explicit Body(pugi::xml_node bodyNode);

        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs();
        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs() const;
        absl::enable_if_t<is_docx_element<Table>::value, ElementRange<Table>> tables();
        absl::enable_if_t<is_docx_element<Table>::value, ElementRange<Table>> tables() const;

        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);
        Table add_table(int rows, int cols);

    private:
        pugi::xml_node m_bodyNode;
        Paragraph m_paragraph;
        Table m_table;
    };
} // namespace duckx
