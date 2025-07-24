/*
 * @file: TextBox.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.08
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include "BaseElement.hpp"
#include "DrawingElement.hpp"
#include "duckxiterator.hpp"
#include "duckx_export.h"

namespace duckx
{
    enum class BorderStyle
    {
        NONE, // 无边框
        SOLID // 实线边框
    };

    class DUCKX_API TextBox : public DrawingElement
    {
    public:
        TextBox();

        explicit TextBox(BorderStyle border);

        void set_border(BorderStyle border);
        void generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                  unsigned int drawing_id) const override;
        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none) const;
        void add_new_paragraph(const std::string& text = "", formatting_flag f = none) const;
        Paragraph last_paragraph() const;
        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs() const;

    private:
        mutable pugi::xml_document m_internal_doc;
        mutable bool m_is_empty = true;
        BorderStyle m_border_style = BorderStyle::NONE;
    };
} // namespace duckx
