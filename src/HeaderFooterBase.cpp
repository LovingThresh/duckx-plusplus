/*!
 * @file HeaderFooterBase.cpp
 * @brief Implementation of base header and footer functionality
 * 
 * Provides common operations for header and footer elements including
 * content management and XML node handling.
 */
#include "HeaderFooterBase.hpp"

namespace duckx
{
    HeaderFooterBase::HeaderFooterBase(const pugi::xml_node rootNode) : m_rootNode(rootNode)
    {
        if (m_rootNode) {
            m_paragraph.set_parent(m_rootNode);
            m_table.set_parent(m_rootNode);
        }
    }

    Paragraph HeaderFooterBase::add_paragraph(const std::string& text, formatting_flag f)
    {
        const pugi::xml_node new_p_node = m_rootNode.append_child("w:p");
        Paragraph new_p(m_rootNode, new_p_node);

        if (!text.empty()) {
            new_p.add_run(text, f);
        }

        return new_p;
    }

    Table HeaderFooterBase::add_table(const int rows, const int cols)
    {
        pugi::xml_node new_tbl_node = m_rootNode.append_child("w:tbl");

        pugi::xml_node tbl_pr_node = new_tbl_node.append_child("w:tblPr");
        pugi::xml_node tbl_borders_node = tbl_pr_node.append_child("w:tblBorders");
        tbl_borders_node.append_child("w:top").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:left").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:bottom").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:right").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:insideH").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:insideV").append_attribute("w:val").set_value("single");

        pugi::xml_node tbl_grid_node = new_tbl_node.append_child("w:tblGrid");
        for (int i = 0; i < cols; ++i) {
            tbl_grid_node.append_child("w:gridCol").append_attribute("w:w").set_value("2390");
        }

        for (int r = 0; r < rows; ++r) {
            pugi::xml_node tr_node = new_tbl_node.append_child("w:tr");
            for (int c = 0; c < cols; ++c) {
                pugi::xml_node tc_node = tr_node.append_child("w:tc");
                tc_node.append_child("w:p");
            }
        }

        return {m_rootNode, new_tbl_node};
    }

    Header::Header(const pugi::xml_node rootNode)
        : HeaderFooterBase(rootNode)
    {}

    Footer::Footer(const pugi::xml_node rootNode)
        : HeaderFooterBase(rootNode)
    {}
}
