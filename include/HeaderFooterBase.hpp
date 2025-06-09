/*
 * @file: HeaderFooterBase.hpp
 * @brief:
 * 
 * @author: liuy
 * @date: 2025.06.09
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once

#include "BaseElement.hpp"

namespace duckx
{
    class HeaderFooterBase
    {
    public:
        virtual ~HeaderFooterBase() = default;

        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);
        Table add_table(int rows, int cols);

    protected:
        explicit HeaderFooterBase(pugi::xml_node rootNode);

        pugi::xml_node m_rootNode; // The <w:hdr> or <w:ftr> node
        Paragraph m_paragraph;
        Table m_table;
    };

    class Header : public HeaderFooterBase
    {
    public:
        explicit Header(pugi::xml_node rootNode);
    };

    class Footer : public HeaderFooterBase
    {
    public:
        explicit Footer(pugi::xml_node rootNode);
    };
}
