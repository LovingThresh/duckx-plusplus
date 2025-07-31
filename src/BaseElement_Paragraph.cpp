/*!
 * @file BaseElement_Paragraph.cpp
 * @brief Implementation of Paragraph class for document paragraphs
 * 
 * Contains implementations for paragraph formatting, text runs,
 * hyperlinks, and style management functionality.
 */
#include "BaseElement_Paragraph.hpp"

#include <cctype>
#include <cstring>

#include "Document.hpp"
#include "HyperlinkManager.hpp"
#include "StyleManager.hpp"

namespace duckx
{
    /*! @brief Convert line spacing to OOXML format */
    long long line_spacing_to_ooxml(const double spacing)
    {
        return static_cast<long long>(spacing * 240.0);
    }

    Paragraph::Paragraph(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

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

    bool Paragraph::has_next() const
    {
        if (!m_currentNode) return false;
        return find_next_sibling("w:p") != nullptr;
    }

    bool Paragraph::has_next_same_type() const
    {
        if (!m_currentNode)
            return false;

        return !find_next_sibling("w:p").empty();
    }

    Paragraph& Paragraph::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:p");
        m_run.set_parent(m_currentNode);
        return *this;
    }

    bool Paragraph::try_advance()
    {
        const pugi::xml_node next_run = find_next_sibling("w:p");
        if (!next_run.empty())
        {
            m_currentNode = next_run;
            return true;
        }
        return false;
    }

    bool Paragraph::can_advance() const
    {
        return !find_next_sibling("w:p").empty();
    }

    bool Paragraph::move_to_next_paragraph()
    {
        const pugi::xml_node next_para = find_next_sibling("w:p");
        if (!next_para)
            return false;

        m_currentNode = next_para;
        m_run.set_parent(m_currentNode);
        return true;
    }

    Alignment Paragraph::get_alignment() const
    {
        const auto pPr = m_currentNode.child("w:pPr");
        if (pPr)
        {
            const auto jc = pPr.child("w:jc");
            if (jc)
            {
                const std::string val = jc.attribute("w:val").as_string();
                if (val == "center") return Alignment::CENTER;
                if (val == "right") return Alignment::RIGHT;
                if (val == "both") return Alignment::BOTH;
            }
        }
        return Alignment::LEFT;
    }

    bool Paragraph::get_line_spacing(double& line_spacing) const
    {
        line_spacing = 1.0; // Default line spacing is 1.0 (single spacing)
        
        const auto pPr = m_currentNode.child("w:pPr");
        if (pPr)
        {
            const auto spacingNode = pPr.child("w:spacing");
            if (spacingNode)
            {
                const auto line = spacingNode.attribute("w:line");
                if (line)
                {
                    line_spacing = line.as_double() / 240.0;
                    return true;
                }
            }
        }
        return false;
    }

    bool Paragraph::get_spacing(double& before_pts, double& after_pts) const
    {
        before_pts = 0.0;
        after_pts = 0.0;
        
        const auto pPr = m_currentNode.child("w:pPr");
        if (pPr)
        {
            const auto spacingNode = pPr.child("w:spacing");
            if (spacingNode)
            {
                bool found = false;
                const auto before = spacingNode.attribute("w:before");
                if (before)
                {
                    before_pts = before.as_double() / 20.0;
                    found = true;
                }
                const auto after = spacingNode.attribute("w:after");
                if (after)
                {
                    after_pts = after.as_double() / 20.0;
                    found = true;
                }
                return found;
            }
        }
        return false;
    }

    bool Paragraph::get_indentation(double& left_pts, double& right_pts, double& first_line_pts) const
    {
        left_pts = 0.0;
        right_pts = 0.0;
        first_line_pts = 0.0;
        
        const auto pPr = m_currentNode.child("w:pPr");
        if (pPr)
        {
            const auto indNode = pPr.child("w:ind");
            if (indNode)
            {
                bool found = false;
                const auto left = indNode.attribute("w:left");
                if (left)
                {
                    left_pts = left.as_double() / 20.0;
                    found = true;
                }
                const auto right = indNode.attribute("w:right");
                if (right)
                {
                    right_pts = right.as_double() / 20.0;
                    found = true;
                }
                const auto firstLine = indNode.attribute("w:firstLine");
                if (firstLine)
                {
                    first_line_pts = firstLine.as_double() / 20.0;
                    found = true;
                }
                const auto hanging = indNode.attribute("w:hanging");
                if (hanging)
                {
                    first_line_pts = -hanging.as_double() / 20.0;
                    found = true;
                }
                return found;
            }
        }
        return false;
    }

    bool Paragraph::get_list_style(ListType& type, int& level, int& numId) const
    {
        const auto pPr = m_currentNode.child("w:pPr");
        if (pPr)
        {
            const auto numPr = pPr.child("w:numPr");
            if (numPr)
            {
                const auto ilvl = numPr.child("w:ilvl");
                if (ilvl)
                {
                    level = ilvl.attribute("w:val").as_int(-1);
                }
                else
                {
                    level = -1;
                }

                const auto id = numPr.child("w:numId");
                if (id)
                {
                    numId = id.attribute("w:val").as_int(-1);
                }
                else
                {
                    numId = -1;
                }

                // As before, we can't easily determine the type (BULLET/NUMBER)
                // without parsing the numbering.xml, so we'll default to BULLET
                type = ListType::BULLET;

                return numId > 0;
            }
        }
        return false;
    }

    absl::enable_if_t<is_docx_element<Run>::value, ElementRange<Run>> Paragraph::runs()
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

        return make_element_range(m_run);
    }

    absl::enable_if_t<is_docx_element<Run>::value, ElementRange<Run>> Paragraph::runs() const
    {
        Run temp_run;

        if (m_currentNode)
        {
            temp_run.set_current(m_currentNode.child("w:r"));
        }
        else
        {
            temp_run.set_current(pugi::xml_node());
        }

        temp_run.set_parent(m_currentNode);

        return make_element_range(temp_run);
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

        if (text != nullptr && *text != 0) {
            // Safe character range check for isspace() - must be in range [0, 255] or EOF (-1)
            unsigned char first_char = static_cast<unsigned char>(text[0]);
            size_t text_len = strlen(text);
            unsigned char last_char = text_len > 0 ? static_cast<unsigned char>(text[text_len - 1]) : 0;
            
            if (isspace(first_char) || (text_len > 0 && isspace(last_char))) {
                new_run_text.append_attribute("xml:space").set_value("preserve");
            }
        }
        new_run_text.text().set(text ? text : "");

        return *new Run(m_currentNode, new_run);
    }

    Run Paragraph::add_hyperlink(const Document& doc, const std::string& text, const std::string& url)
    {
        const HyperlinkManager& link_manager = doc.links();

        const std::string rId = link_manager.add_relationship(url);

        pugi::xml_node hyperlink_node = m_currentNode.append_child("w:hyperlink");
        hyperlink_node.append_attribute("r:id").set_value(rId.c_str());

        pugi::xml_node run_node = hyperlink_node.append_child("w:r");

        pugi::xml_node rpr_node = run_node.append_child("w:rPr");

        rpr_node.append_child("w:rStyle").append_attribute("w:val").set_value("Hyperlink");

        pugi::xml_node color_node = rpr_node.append_child("w:color");
        color_node.append_attribute("w:val").set_value("0563C1");

        pugi::xml_node underline_node = rpr_node.append_child("w:u");
        underline_node.append_attribute("w:val").set_value("single");

        pugi::xml_node text_node = run_node.append_child("w:t");
        text_node.text().set(text.c_str());
        if (!text.empty() && (isspace(text.front()) || isspace(text.back())))
        {
            text_node.append_attribute("xml:space").set_value("preserve");
        }

        return {hyperlink_node, run_node};
    }

    Paragraph& Paragraph::set_alignment(const Alignment align)
    {
        pugi::xml_node pPr_node = get_or_create_pPr();

        // 2. 查找或创建 <w:jc> (justification) 节点
        pugi::xml_node jc_node = pPr_node.child("w:jc");
        if (!jc_node)
        {
            jc_node = pPr_node.append_child("w:jc");
        }

        auto align_str = "left";
        switch (align)
        {
            case Alignment::LEFT:
                align_str = "left";
                break;
            case Alignment::CENTER:
                align_str = "center";
                break;
            case Alignment::RIGHT:
                align_str = "right";
                break;
            case Alignment::BOTH:
                align_str = "both";
                break;
        }

        pugi::xml_attribute val_attr = jc_node.attribute("w:val");
        if (!val_attr)
        {
            val_attr = jc_node.append_attribute("w:val");
        }
        val_attr.set_value(align_str);

        return *this;
    }

    Paragraph& Paragraph::set_spacing(const double before_pts, const double after_pts)
    {
        pugi::xml_node pPr_node = get_or_create_pPr();
        pugi::xml_node spacing_node = pPr_node.child("w:spacing");
        if (!spacing_node)
        {
            spacing_node = pPr_node.append_child("w:spacing");
        }

        if (before_pts >= 0)
        {
            // 更新现有属性而不是重复添加
            pugi::xml_attribute before_attr = spacing_node.attribute("w:before");
            if (!before_attr) {
                before_attr = spacing_node.append_attribute("w:before");
            }
            before_attr.set_value(points_to_twips(before_pts));
        }
        if (after_pts >= 0)
        {
            pugi::xml_attribute after_attr = spacing_node.attribute("w:after");
            if (!after_attr) {
                after_attr = spacing_node.append_attribute("w:after");
            }
            after_attr.set_value(points_to_twips(after_pts));
        }

        return *this;
    }

    Paragraph& Paragraph::set_line_spacing(const double line_spacing)
    {
        pugi::xml_node pPr_node = get_or_create_pPr();
        pugi::xml_node spacing_node = pPr_node.child("w:spacing");
        if (!spacing_node)
        {
            spacing_node = pPr_node.append_child("w:spacing");
        }
        // 更新现有属性而不是重复添加
        pugi::xml_attribute line_attr = spacing_node.attribute("w:line");
        if (!line_attr) {
            line_attr = spacing_node.append_attribute("w:line");
        }
        line_attr.set_value(line_spacing_to_ooxml(line_spacing));
        return *this;
    }

    Paragraph& Paragraph::set_indentation(const double left_pts, const double right_pts)
    {
        pugi::xml_node pPr_node = get_or_create_pPr();
        pugi::xml_node ind_node = pPr_node.child("w:ind");
        if (!ind_node)
        {
            ind_node = pPr_node.append_child("w:ind");
        }

        if (left_pts >= 0)
        {
            ind_node.append_attribute("w:left").set_value(points_to_twips(left_pts));
        }
        if (right_pts >= 0)
        {
            ind_node.append_attribute("w:right").set_value(points_to_twips(right_pts));
        }

        return *this;
    }

    Paragraph& Paragraph::set_first_line_indent(const double first_line_pts)
    {
        pugi::xml_node pPr_node = get_or_create_pPr();
        pugi::xml_node ind_node = pPr_node.child("w:ind");
        if (!ind_node)
        {
            ind_node = pPr_node.append_child("w:ind");
        }

        if (first_line_pts > 0)
        {
            // 首行缩进
            ind_node.append_attribute("w:firstLine").set_value(points_to_twips(first_line_pts));
            ind_node.remove_attribute("w:hanging"); // 确保与悬挂缩进互斥
        }
        else if (first_line_pts < 0)
        {
            // 悬挂缩进
            ind_node.append_attribute("w:hanging").set_value(points_to_twips(-first_line_pts));
            ind_node.remove_attribute("w:firstLine"); // 确保与首行缩进互斥
        }
        else
        {
            // == 0
            // 移除缩进
            ind_node.remove_attribute("w:firstLine");
            ind_node.remove_attribute("w:hanging");
        }

        return *this;
    }

    Paragraph& Paragraph::set_list_style(const ListType type, const int level)
    {
        pugi::xml_node pPr_node = get_or_create_pPr();
        pugi::xml_node numPr_node = pPr_node.child("w:numPr");

        if (type == ListType::NONE)
        {
            // 如果要移除列表样式，则删除整个 <w:numPr> 节点
            if (numPr_node)
            {
                pPr_node.remove_child(numPr_node);
            }
            return *this;
        }

        // 如果 <w:numPr> 不存在，则创建一个
        if (!numPr_node)
        {
            numPr_node = pPr_node.append_child("w:numPr");
        }

        // --- 设置层级 <w:ilvl> ---
        pugi::xml_node ilvl_node = numPr_node.child("w:ilvl");
        if (!ilvl_node)
        {
            ilvl_node = numPr_node.append_child("w:ilvl");
        }
        ilvl_node.append_attribute("w:val").set_value(level);

        // --- 设置列表ID <w:numId> ---
        pugi::xml_node numId_node = numPr_node.child("w:numId");
        if (!numId_node)
        {
            numId_node = numPr_node.append_child("w:numId");
        }

        // 根据类型，引用我们在 numbering.xml 中预定义的 ID
        const int list_id = (type == ListType::BULLET) ? 1 : 2;
        numId_node.append_attribute("w:val").set_value(list_id);

        return *this;
    }

    Paragraph& Paragraph::insert_paragraph_after(const std::string& text, formatting_flag f)
    {
        const pugi::xml_node new_para = m_parentNode.insert_child_after("w:p", m_currentNode);

        const auto p = new Paragraph();
        p->set_current(new_para);
        p->add_run(text, f);

        return *p;
    }

    pugi::xml_node Paragraph::get_or_create_pPr()
    {
        pugi::xml_node pPr_node = m_currentNode.child("w:pPr");
        if (!pPr_node)
        {
            // Try to insert before first child, but if that fails, just append
            pugi::xml_node first_child = m_currentNode.first_child();
            if (first_child) {
                pPr_node = m_currentNode.insert_child_before("w:pPr", first_child);
            }
            
            // If insert_child_before failed or there was no first child, append
            if (!pPr_node) {
                pPr_node = m_currentNode.append_child("w:pPr");
            }
        }
        return pPr_node;
    }

    // Modern Result<T> API for style application
    Result<void> Paragraph::apply_style_safe(const StyleManager& style_manager, const std::string& style_name)
    {
        // Verify style exists and is compatible
        auto style_result = style_manager.get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        const Style* style = style_result.value();
        if (style->type() != StyleType::PARAGRAPH && style->type() != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Style '%s' is not a paragraph or mixed style", style_name),
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        // Get or create paragraph properties node
        pugi::xml_node ppr = get_or_create_pPr();
        if (!ppr) {
            return Result<void>(errors::xml_manipulation_failed(
                "Failed to create paragraph properties node",
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        // Apply style reference
        pugi::xml_node style_ref = ppr.child("w:pStyle");
        if (!style_ref) {
            style_ref = ppr.append_child("w:pStyle");
        }
        style_ref.attribute("w:val") ? 
            style_ref.attribute("w:val").set_value(style_name.c_str()) :
            style_ref.append_attribute("w:val").set_value(style_name.c_str());
        
        return Result<void>{};
    }

    Result<std::string> Paragraph::get_style_safe() const
    {
        pugi::xml_node ppr = m_currentNode.child("w:pPr");
        if (!ppr) {
            return Result<std::string>{std::string{}}; // No style applied
        }
        
        pugi::xml_node style_ref = ppr.child("w:pStyle");
        if (!style_ref) {
            return Result<std::string>{std::string{}}; // No style applied
        }
        
        pugi::xml_attribute val_attr = style_ref.attribute("w:val");
        if (!val_attr) {
            return Result<std::string>{std::string{}}; // No style value
        }
        
        return Result<std::string>{std::string(val_attr.value())};
    }

    Result<void> Paragraph::remove_style_safe()
    {
        pugi::xml_node ppr = m_currentNode.child("w:pPr");
        if (!ppr) {
            return Result<void>{}; // No properties to remove
        }
        
        pugi::xml_node style_ref = ppr.child("w:pStyle");
        if (style_ref) {
            ppr.remove_child(style_ref);
        }
        
        return Result<void>{};
    }

} // namespace duckx