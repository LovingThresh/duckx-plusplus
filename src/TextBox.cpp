/*
 * @file: TextBox.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.08
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "TextBox.hpp"

namespace duckx
{
    TextBox::TextBox()
    {
        m_internal_doc.append_child("w:txbxContent");
    }

    TextBox::TextBox(const BorderStyle border)
        : TextBox()
    {
        m_border_style = border;
    }

    void TextBox::set_border(const BorderStyle border)
    {
        m_border_style = border;
    }

    void TextBox::generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                       const unsigned int drawing_id) const
    {
        (void)relationship_id;

        auto drawing = parent_run_node.append_child("w:drawing");

        pugi::xml_node drawing_root;
        const std::string drawing_id_str = std::to_string(drawing_id);
        const std::string textbox_name = "Text Box " + drawing_id_str;
        const std::string width_str = std::to_string(m_width_emu);
        const std::string height_str = std::to_string(m_height_emu);

        if (m_has_position)
        {
            drawing_root = drawing.append_child("wp:anchor");
            auto positionH = drawing_root.append_child("wp:positionH");
            const char* h_rel_str = (m_h_relative_from == RelativeFrom::PAGE) ? "page" : "margin";
            positionH.append_attribute("relativeFrom").set_value(h_rel_str);
            positionH.append_child("wp:posOffset").text().set(std::to_string(m_pos_x_emu).c_str());
            auto positionV = drawing_root.append_child("wp:positionV");
            const char* v_rel_str = (m_v_relative_from == RelativeFrom::PAGE) ? "page" : "margin";
            positionV.append_attribute("relativeFrom").set_value(v_rel_str);
            positionV.append_child("wp:posOffset").text().set(std::to_string(m_pos_y_emu).c_str());
            auto simplePos = drawing_root.append_child("wp:simplePos");
            simplePos.append_attribute("x").set_value("0");
            simplePos.append_attribute("y").set_value("0");
            drawing_root.append_child("wp:wrapNone");
            drawing_root.append_attribute("distT").set_value("0");
            drawing_root.append_attribute("distB").set_value("0");
            drawing_root.append_attribute("distL").set_value("0");
            drawing_root.append_attribute("distR").set_value("0");
            drawing_root.append_attribute("behindDoc").set_value("0");
            drawing_root.append_attribute("locked").set_value("0");
            drawing_root.append_attribute("layoutInCell").set_value("1");
            drawing_root.append_attribute("allowOverlap").set_value("1");
        }
        else
        {
            drawing_root = drawing.append_child("wp:inline");
            drawing_root.append_attribute("distT").set_value("0");
            drawing_root.append_attribute("distB").set_value("0");
            drawing_root.append_attribute("distL").set_value("0");
            drawing_root.append_attribute("distR").set_value("0");
        }

        auto extent = drawing_root.append_child("wp:extent");
        extent.append_attribute("cx").set_value(width_str.c_str());
        extent.append_attribute("cy").set_value(height_str.c_str());

        auto effectExtent = drawing_root.append_child("wp:effectExtent");
        effectExtent.append_attribute("l").set_value("0");
        effectExtent.append_attribute("t").set_value("0");
        effectExtent.append_attribute("r").set_value("0");
        effectExtent.append_attribute("b").set_value("0");

        if (m_has_position)
        {
            // Simple Position (通常设为0,0)
            auto simplePos = drawing_root.append_child("wp:simplePos");
            simplePos.append_attribute("x").set_value("0");
            simplePos.append_attribute("y").set_value("0");

            // Horizontal Position
            auto positionH = drawing_root.append_child("wp:positionH");
            const char* h_rel_str = (m_h_relative_from == RelativeFrom::PAGE) ? "page" : "margin";
            positionH.append_attribute("relativeFrom").set_value(h_rel_str);
            positionH.append_child("wp:posOffset").text().set(std::to_string(m_pos_x_emu).c_str());

            // Vertical Position
            auto positionV = drawing_root.append_child("wp:positionV");
            const char* v_rel_str = (m_v_relative_from == RelativeFrom::PAGE) ? "page" : "margin";
            positionV.append_attribute("relativeFrom").set_value(v_rel_str);
            positionV.append_child("wp:posOffset").text().set(std::to_string(m_pos_y_emu).c_str());
        }

        auto docPr = drawing_root.append_child("wp:docPr");
        docPr.append_attribute("id").set_value(drawing_id_str.c_str());
        docPr.append_attribute("name").set_value(textbox_name.c_str());

        drawing_root.append_child("wp:cNvGraphicFramePr");

        auto graphic = drawing_root.append_child("a:graphic");
        graphic.append_attribute("xmlns:a").set_value("http://schemas.openxmlformats.org/drawingml/2006/main");

        auto graphicData = graphic.append_child("a:graphicData");
        graphicData.append_attribute("uri").set_value(
                "http://schemas.microsoft.com/office/word/2010/wordprocessingShape");

        auto wsp = graphicData.append_child("wps:wsp");

        auto cNvPr = wsp.append_child("wps:cNvPr");
        cNvPr.append_attribute("id").set_value(std::to_string(drawing_id + 1).c_str());
        cNvPr.append_attribute("name").set_value(textbox_name.c_str());
        wsp.append_child("wps:cNvSpPr");

        wsp.append_child(pugi::node_pcdata).set_value("\n");

        auto spPr = wsp.append_child("wps:spPr");
        auto xfrm = spPr.append_child("a:xfrm");
        xfrm.append_child("a:off").append_attribute("x").set_value("0");
        xfrm.child("a:off").append_attribute("y").set_value("0");
        xfrm.append_child("a:ext").append_attribute("cx").set_value(std::to_string(m_width_emu).c_str());
        xfrm.child("a:ext").append_attribute("cy").set_value(std::to_string(m_height_emu).c_str());

        auto prstGeom = spPr.append_child("a:prstGeom");
        prstGeom.append_attribute("prst").set_value("rect");
        prstGeom.append_child("a:avLst");

        spPr.append_child("a:noFill");
        auto ln = spPr.append_child("a:ln");
        if (m_border_style == BorderStyle::SOLID)
        {
            auto solidFill = ln.append_child("a:solidFill");
            auto srgbClr = solidFill.append_child("a:srgbClr");
            srgbClr.append_attribute("val").set_value("000000");
        }
        else
        {
            ln.append_child("a:noFill");
        }

        auto txbx = wsp.append_child("wps:txbx");
        pugi::xml_node target_content_node = txbx.append_child("w:txbxContent");
        const pugi::xml_node source_content_node = m_internal_doc.child("w:txbxContent");
        auto bodyPr = wsp.append_child("wps:bodyPr");
        bodyPr.append_attribute("wrap").set_value("square");
        bodyPr.append_attribute("lIns").set_value("91440");
        bodyPr.append_attribute("tIns").set_value("45720");
        bodyPr.append_attribute("rIns").set_value("91440");
        bodyPr.append_attribute("bIns").set_value("45720");

        for (pugi::xml_node p_node: source_content_node.children("w:p"))
        {
            target_content_node.append_copy(p_node);
        }
        if (!target_content_node.first_child())
        {
            target_content_node.append_child("w:p");
        }

        if (m_has_position)
        {
            drawing_root.append_child("wp:wrapNone");
        }
    }

    Paragraph TextBox::add_paragraph(const std::string& text, formatting_flag f) const
    {
        pugi::xml_node content_node = m_internal_doc.child("w:txbxContent");

        if (m_is_empty && content_node.first_child() && content_node.first_child().first_child() == nullptr) {
            content_node.remove_child(content_node.first_child());
        }
        m_is_empty = false;

        const pugi::xml_node p_node = content_node.append_child("w:p");

        Paragraph new_para(content_node, p_node);

        if (!text.empty())
        {
            new_para.add_run(text);
        }

        return new_para;
    }

    void TextBox::add_new_paragraph(const std::string& text, formatting_flag f) const
    {
        (void)add_paragraph(text);
    }

    Paragraph TextBox::last_paragraph() const
    {
        pugi::xml_node content_node = m_internal_doc.child("w:txbxContent");
        pugi::xml_node last_p_node = content_node.last_child();
        if (last_p_node && strcmp(last_p_node.name(), "w:p") == 0)
        {
            return {content_node, last_p_node};
        }
        return {};
    }

    ElementRange<Paragraph> TextBox::paragraphs() const
    {
        const pugi::xml_node content_node = m_internal_doc.child("w:txbxContent");

        Paragraph p_state;
        p_state.set_parent(content_node);
        p_state.set_current(content_node.child("w:p"));
        return ElementRange<Paragraph>(p_state);
    }
} // namespace duckx
