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

    Table Body::add_table(const int rows, const int cols)
    {
        // 1. 创建表格主节点 <w:tbl>
        pugi::xml_node new_tbl_node = m_bodyNode.append_child("w:tbl");

        // 2. 添加表格属性 <w:tblPr> 和默认边框，使其可见
        pugi::xml_node tbl_pr_node = new_tbl_node.append_child("w:tblPr");
        pugi::xml_node tbl_borders_node = tbl_pr_node.append_child("w:tblBorders");
        tbl_borders_node.append_child("w:top").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:left").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:bottom").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:right").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:insideH").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:insideV").append_attribute("w:val").set_value("single");

        // 3. 定义表格网格 <w:tblGrid>
        pugi::xml_node tbl_grid_node = new_tbl_node.append_child("w:tblGrid");
        for (int i = 0; i < cols; ++i) {
            // 为每一列定义一个 gridCol，并设置默认宽度
            tbl_grid_node.append_child("w:gridCol").append_attribute("w:w").set_value("2390");
        }

        // 4. 创建行 <w:tr> 和单元格 <w:tc>
        for (int r = 0; r < rows; ++r) {
            pugi::xml_node tr_node = new_tbl_node.append_child("w:tr");
            for (int c = 0; c < cols; ++c) {
                pugi::xml_node tc_node = tr_node.append_child("w:tc");
                // 关键：每个单元格必须包含一个空段落 <w:p>
                tc_node.append_child("w:p");
            }
        }

        // 5. 返回一个指向新创建的表格的 Table 对象
        return {m_bodyNode, new_tbl_node};
    }
} // namespace duckx
