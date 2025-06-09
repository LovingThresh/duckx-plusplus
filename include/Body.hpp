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

namespace duckx
{
    class Body
    {
    public:
        Body() = default;
        explicit Body(pugi::xml_node bodyNode);

        ElementRange<Paragraph> paragraphs();
        ElementRange<Table> tables();

        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);
        Table add_table(int rows, int cols);

    private:
        pugi::xml_node m_bodyNode;
        Paragraph m_paragraph;
        Table m_table;
    };
} // namespace duckx
