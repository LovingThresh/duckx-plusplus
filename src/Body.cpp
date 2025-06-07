/*
 * @file: Body.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "../include/Body.hpp"
namespace duckx
{
    Body::Body(const pugi::xml_node bodyNode) : m_bodyNode(bodyNode)
    {
        if (m_bodyNode)
        {
            m_paragraph.set_parent(m_bodyNode);
            m_paragraph.set_current(m_bodyNode.child("w:p"));
            m_table.set_parent(m_bodyNode);
            m_table.set_current(m_bodyNode.child("w:tbl"));
        }
    }

    ElementRange<Paragraph> Body::paragraphs()
    {
        if (m_bodyNode)
        {
            m_paragraph.set_current(m_bodyNode.child("w:p"));
        }
        else
        {
            m_paragraph.set_current(pugi::xml_node());
        }

        m_paragraph.set_parent(m_bodyNode);

        return ElementRange<Paragraph>(m_paragraph);
    }

    ElementRange<Table> Body::tables()
    {
        if (m_bodyNode)
        {
            m_table.set_current(m_bodyNode.child("w:tbl"));
        }
        else
        {
            m_table.set_current(pugi::xml_node());
        }

        m_table.set_parent(m_bodyNode);

        return ElementRange<Table>(m_table);
    }

    Paragraph Body::add_paragraph(const std::string& text, const formatting_flag f)
    {
        const pugi::xml_node pNode = m_bodyNode.append_child("w:p");
        Paragraph newPara(m_bodyNode, pNode);
        if (!text.empty())
        {
            newPara.add_run(text, f);
        }
        return newPara;
    }

    Table Body::add_table(int rows, int cols)
    {
        const pugi::xml_node tableNode = m_bodyNode.append_child("w:tbl");
        return {m_bodyNode, tableNode};
    }
} // namespace duckx
