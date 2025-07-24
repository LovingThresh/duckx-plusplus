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
#include <map>
#include <tuple>
#include <unordered_map>
#include <zip.h>

#include "Document.hpp"
#include "HyperlinkManager.hpp"

namespace duckx
{
    std::unordered_map<std::string, int> node_map = {
        {"w:p", 0},
        {"w:tbl", 1},
        {"w:r", 2},
        {"w:tr", 3},
        {"w:tc", 4}
    };

    long long points_to_twips(const double pts)
    {
        return static_cast<long long>(pts * 20.0);
    }

    long long line_spacing_to_ooxml(const double spacing)
    {
        return static_cast<long long>(spacing * 240.0);
    }

    static std::string highlight_color_to_string(const HighlightColor color)
    {
        static const std::map<HighlightColor, std::string> color_map = {
            {HighlightColor::BLACK, "black"},
            {HighlightColor::BLUE, "blue"},
            {HighlightColor::CYAN, "cyan"},
            {HighlightColor::GREEN, "green"},
            {HighlightColor::MAGENTA, "magenta"},
            {HighlightColor::RED, "red"},
            {HighlightColor::YELLOW, "yellow"},
            {HighlightColor::WHITE, "white"},
            {HighlightColor::DARK_BLUE, "darkBlue"},
            {HighlightColor::DARK_CYAN, "darkCyan"},
            {HighlightColor::DARK_GREEN, "darkGreen"},
            {HighlightColor::DARK_MAGENTA, "darkMagenta"},
            {HighlightColor::DARK_RED, "darkRed"},
            {HighlightColor::DARK_YELLOW, "darkYellow"},
            {HighlightColor::LIGHT_GRAY, "lightGray"}
        };

        const auto it = color_map.find(color);
        if (it != color_map.end())
        {
            return it->second;
        }
        return ""; // Return an empty string if not found
    }

    static bool check_boolean_property(const pugi::xml_node& rPr, const char* tag_name)
    {
        if (!rPr) return false;

        const auto tag = rPr.child(tag_name);
        if (!tag) return false;

        // The property is present. Now check if it's explicitly set to false.
        const auto val_attr = tag.attribute("w:val");
        if (val_attr && (std::string(val_attr.as_string()) == "false" || std::string(val_attr.as_string()) == "0"))
        {
            return false;
        }

        return true;
    }

    DocxElement::DocxElement(const pugi::xml_node parentNode, const pugi::xml_node currentNode)
        : m_parentNode(parentNode), m_currentNode(currentNode) {}

    pugi::xml_node DocxElement::get_node() const
    {
        return m_currentNode;
    }

    bool DocxElement::has_next_sibling() const
    {
        return !find_next_any_sibling().empty();
    }

    DocxElement::SiblingInfo DocxElement::peek_next_sibling() const
    {
        const pugi::xml_node next = find_next_any_sibling();
        if (!next)
        {
            return {};
        }

        const ElementType type = determine_element_type(next);
        const std::string tag_name = next.name();

        return {type, tag_name};
    }

    pugi::xml_node DocxElement::find_next_sibling(const std::string& name) const
    {
        const pugi::xml_node sibling = m_currentNode.next_sibling(name.c_str());
        return sibling;
    }

    pugi::xml_node DocxElement::find_next_any_sibling() const
    {
        if (!m_currentNode)
            return {};

        return m_currentNode.next_sibling();
    }

    DocxElement::ElementType DocxElement::map_string_to_element_type(const std::string& node_name)
    {
        const auto it = node_map.find(node_name);
        const int type = (it != node_map.end()) ? it->second : -1;
        switch (type)
        {
            case 0: return ElementType::PARAGRAPH;
            case 1: return ElementType::TABLE;
            case 2: return ElementType::RUN;
            case 3: return ElementType::TABLE_ROW;
            case 4: return ElementType::TABLE_CELL;
            default: return ElementType::UNKNOWN;
        }
    }

    DocxElement::ElementType DocxElement::determine_element_type(const pugi::xml_node node)
    {
        if (!node)
            return ElementType::UNKNOWN;

        const std::string node_name = node.name();

        if (node_name == "w:p")
            return ElementType::PARAGRAPH;
        else if (node_name == "w:tbl")
            return ElementType::TABLE;
        else if (node_name == "w:r")
            return ElementType::RUN;
        else if (node_name == "w:tr")
            return ElementType::TABLE_ROW;
        else if (node_name == "w:tc")
            return ElementType::TABLE_CELL;
        else
            return ElementType::UNKNOWN;
    }

    Run::Run(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

    bool Run::has_next() const
    {
        if (!m_currentNode) return false;
        return find_next_sibling("w:r") != nullptr;
    }

    bool Run::has_next_same_type() const
    {
        if (!m_currentNode)
            return false;

        return !find_next_sibling("w:r").empty();
    }

    void Run::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:r");
    }

    void Run::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    std::string Run::get_text() const
    {
        return m_currentNode.child("w:t").text().get();
    }

    bool Run::is_bold() const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        return check_boolean_property(rPr, "w:b");
    }

    bool Run::is_italic() const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        return check_boolean_property(rPr, "w:i");
    }

    bool Run::is_underline() const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        if (!rPr) return false;

        const auto tag = rPr.child("w:u");
        if (!tag) return false;

        // Underline can be turned off with w:val="none"
        const auto val_attr = tag.attribute("w:val");
        if (val_attr && std::string(val_attr.as_string()) == "none")
        {
            return false;
        }

        return true;
    }

    bool Run::get_font(std::string& font_name) const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        if (!rPr) return false;

        const auto rFonts = rPr.child("w:rFonts");
        if (!rFonts) return false;

        // The "ascii" attribute is typically the one to use for standard characters.
        const auto font_attr = rFonts.attribute("w:ascii");
        if (font_attr)
        {
            font_name = font_attr.as_string();
            return true;
        }

        return false;
    }

    bool Run::get_font_size(double& size) const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        if (!rPr) return false;

        const auto sz = rPr.child("w:sz");
        if (!sz) return false;

        const auto val_attr = sz.attribute("w:val");
        if (val_attr)
        {
            // Font size in docx is stored in "half-points"
            size = val_attr.as_double() / 2.0;
            return true;
        }

        return false;
    }

    bool Run::get_color(std::string& color) const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        if (!rPr) return false;

        const auto color_node = rPr.child("w:color");
        if (!color_node) return false;

        const auto val_attr = color_node.attribute("w:val");
        if (val_attr)
        {
            color = val_attr.as_string();
            // Don't return "auto" colors as they are not explicit RGB values
            if (color == "auto") return false;
            return true;
        }

        return false;
    }

    bool Run::get_highlight(HighlightColor& color) const
    {
        const auto rPr = m_currentNode.child("w:rPr");
        if (!rPr) return false;

        const auto highlight_node = rPr.child("w:highlight");
        if (!highlight_node) return false;

        const auto val_attr = highlight_node.attribute("w:val");
        if (!val_attr) return false;

        const std::string val_str = val_attr.as_string();

        static const std::map<std::string, HighlightColor> color_map = {
            {"black", HighlightColor::BLACK}, {"blue", HighlightColor::BLUE},
            {"cyan", HighlightColor::CYAN}, {"green", HighlightColor::GREEN},
            {"magenta", HighlightColor::MAGENTA}, {"red", HighlightColor::RED},
            {"yellow", HighlightColor::YELLOW}, {"white", HighlightColor::WHITE},
            {"darkBlue", HighlightColor::DARK_BLUE}, {"darkCyan", HighlightColor::DARK_CYAN},
            {"darkGreen", HighlightColor::DARK_GREEN}, {"darkMagenta", HighlightColor::DARK_MAGENTA},
            {"darkRed", HighlightColor::DARK_RED}, {"darkYellow", HighlightColor::DARK_YELLOW},
            {"lightGray", HighlightColor::LIGHT_GRAY}
        };

        const auto it = color_map.find(val_str);
        if (it != color_map.end())
        {
            color = it->second;
            return true;
        }

        return false;
    }

    formatting_flag Run::get_formatting() const
    {
        formatting_flag flag = none;
        if (is_bold()) flag |= bold;
        if (is_italic()) flag |= italic;
        if (is_underline()) flag |= underline;

        const auto rPr = m_currentNode.child("w:rPr");
        if (rPr)
        {
            if (check_boolean_property(rPr, "w:strike")) flag |= strikethrough;
            if (check_boolean_property(rPr, "w:smallCaps")) flag |= smallcaps;
            if (check_boolean_property(rPr, "w:shadow")) flag |= shadow;

            if (const auto vertAlign = rPr.child("w:vertAlign"))
            {
                const std::string val = vertAlign.attribute("w:val").as_string();
                if (val == "superscript") flag |= superscript;
                else if (val == "subscript") flag |= subscript;
            }
        }

        return flag;
    }

    bool Run::set_text(const std::string& text) const
    {
        return m_currentNode.child("w:t").text().set(text.c_str());
    }

    bool Run::set_text(const char* text) const
    {
        return m_currentNode.child("w:t").text().set(text);
    }

    Run& Run::set_font(const std::string& font_name)
    {
        pugi::xml_node rPr_node = get_or_create_rPr();
        pugi::xml_node rFonts_node = rPr_node.child("w:rFonts");
        if (!rFonts_node)
        {
            rFonts_node = rPr_node.append_child("w:rFonts");
        }

        // 设置所有字体类型，以确保在各种情况下都能正确显示
        rFonts_node.append_attribute("w:ascii").set_value(font_name.c_str());
        rFonts_node.append_attribute("w:hAnsi").set_value(font_name.c_str());
        rFonts_node.append_attribute("w:eastAsia").set_value(font_name.c_str());
        rFonts_node.append_attribute("w:cs").set_value(font_name.c_str());

        return *this;
    }

    Run& Run::set_font_size(const double size)
    {
        pugi::xml_node rPr_node = get_or_create_rPr();
        pugi::xml_node sz_node = rPr_node.child("w:sz");
        if (!sz_node)
        {
            sz_node = rPr_node.append_child("w:sz");
        }

        // 字号单位是 "half-points"，所以需要乘以2
        const int half_points = static_cast<int>(std::round(size * 2.0));
        sz_node.append_attribute("w:val").set_value(std::to_string(half_points).c_str());

        // 还有一个 <w:szCs> 节点用于复杂字符（如亚洲语言），最好也设置一下
        pugi::xml_node szCs_node = rPr_node.child("w:szCs");
        if (!szCs_node)
        {
            szCs_node = rPr_node.append_child("w:szCs");
        }
        szCs_node.append_attribute("w:val").set_value(std::to_string(half_points).c_str());

        return *this;
    }

    Run& Run::set_color(const std::string& color)
    {
        pugi::xml_node rPr_node = get_or_create_rPr();
        pugi::xml_node color_node = rPr_node.child("w:color");
        if (!color_node)
        {
            color_node = rPr_node.append_child("w:color");
        }

        color_node.append_attribute("w:val").set_value(color.c_str());

        return *this;
    }

    Run& Run::set_highlight(const HighlightColor color)
    {
        pugi::xml_node rPr = get_or_create_rPr();

        pugi::xml_node highlight_node = rPr.child("w:highlight");

        if (color == HighlightColor::NONE)
        {
            if (highlight_node)
            {
                rPr.remove_child(highlight_node);
            }
        }
        else
        {
            const std::string color_str = highlight_color_to_string(color);
            if (!color_str.empty())
            {
                if (!highlight_node)
                {
                    highlight_node = rPr.append_child("w:highlight");
                }

                pugi::xml_attribute val_attr = highlight_node.attribute("w:val");
                if (!val_attr)
                {
                    val_attr = highlight_node.append_attribute("w:val");
                }
                val_attr.set_value(color_str.c_str());
            }
        }

        return *this;
    }

    Run& Run::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:r");
        return *this;
    }

    bool Run::try_advance()
    {
        const pugi::xml_node next_run = find_next_sibling("w:r");
        if (!next_run.empty())
        {
            m_currentNode = next_run;
            return true;
        }
        return false;
    }

    bool Run::can_advance() const
    {
        return !find_next_sibling("w:r").empty();
    }

    bool Run::move_to_next_run()
    {
        const pugi::xml_node next_run = find_next_sibling("w:r");
        if (!next_run)
            return false;

        m_currentNode = next_run;
        return true;
    }

    pugi::xml_node Run::get_or_create_rPr()
    {
        pugi::xml_node rPr_node = m_currentNode.child("w:rPr");
        if (!rPr_node)
        {
            rPr_node = m_currentNode.insert_child_before("w:rPr", m_currentNode.first_child());
        }
        return rPr_node;
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
            spacing_node.append_attribute("w:before").set_value(points_to_twips(before_pts));
        }
        if (after_pts >= 0)
        {
            spacing_node.append_attribute("w:after").set_value(points_to_twips(after_pts));
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
        spacing_node.append_attribute("w:line").set_value(line_spacing_to_ooxml(line_spacing));
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
            pPr_node = m_currentNode.insert_child_before("w:pPr", m_currentNode.first_child());
        }
        return pPr_node;
    }

    TableCell::TableCell(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

    void TableCell::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tc");

        m_paragraph.set_parent(m_currentNode);
    }

    void TableCell::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    bool TableCell::has_next() const
    {
        if (!m_currentNode) return false;
        return find_next_sibling("w:tc") != nullptr;
    }

    bool TableCell::has_next_same_type() const
    {
        if (!m_currentNode)
            return false;

        return !find_next_sibling("w:tc").empty();
    }

    absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> TableCell::paragraphs()
    {
        if (m_currentNode)
        {
            m_paragraph.set_current(m_currentNode.child("w:p"));
        }
        else
        {
            m_paragraph.set_current(pugi::xml_node());
        }

        m_paragraph.set_parent(m_currentNode);

        return make_element_range(m_paragraph);
    }

    TableCell& TableCell::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:tc");
        return *this;
    }

    bool TableCell::try_advance()
    {
        const pugi::xml_node next_cell = find_next_sibling("w:tc");
        if (!next_cell.empty())
        {
            m_currentNode = next_cell;
            return true;
        }
        return false;
    }

    bool TableCell::can_advance() const
    {
        return !find_next_sibling("w:tc").empty();
    }

    bool TableCell::move_to_next_cell()
    {
        m_currentNode = find_next_sibling("w:tc");
        m_paragraph.set_parent(m_currentNode);
        return true;
    }

    Paragraph TableCell::add_paragraph(const std::string& text, formatting_flag f)
    {
        const pugi::xml_node p_node = m_currentNode.append_child("w:p");
        Paragraph new_para(m_currentNode, p_node);
        
        if (!text.empty()) {
            new_para.add_run(text, f);
        }
        
        return new_para;
    }

    TableCell& TableCell::set_width(double width_pts)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_w = tc_pr.child("w:tcW");
        if (!tc_w) {
            tc_w = tc_pr.append_child("w:tcW");
        }
        
        const long long width_twips = static_cast<long long>(width_pts * 20.0);
        tc_w.attribute("w:w") ? tc_w.attribute("w:w").set_value(std::to_string(width_twips).c_str())
                              : tc_w.append_attribute("w:w").set_value(std::to_string(width_twips).c_str());
        tc_w.attribute("w:type") ? tc_w.attribute("w:type").set_value("dxa")
                                 : tc_w.append_attribute("w:type").set_value("dxa");
        return *this;
    }

    TableCell& TableCell::set_width_type(const std::string& type)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_w = tc_pr.child("w:tcW");
        if (!tc_w) {
            tc_w = tc_pr.append_child("w:tcW");
        }
        
        tc_w.attribute("w:type") ? tc_w.attribute("w:type").set_value(type.c_str())
                                 : tc_w.append_attribute("w:type").set_value(type.c_str());
        return *this;
    }

    TableCell& TableCell::set_vertical_alignment(const std::string& align)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node v_align = tc_pr.child("w:vAlign");
        if (!v_align) {
            v_align = tc_pr.append_child("w:vAlign");
        }
        
        v_align.attribute("w:val") ? v_align.attribute("w:val").set_value(align.c_str())
                                   : v_align.append_attribute("w:val").set_value(align.c_str());
        return *this;
    }

    TableCell& TableCell::set_background_color(const std::string& color)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node shd = tc_pr.child("w:shd");
        if (!shd) {
            shd = tc_pr.append_child("w:shd");
        }
        
        shd.attribute("w:val") ? shd.attribute("w:val").set_value("clear")
                               : shd.append_attribute("w:val").set_value("clear");
        shd.attribute("w:color") ? shd.attribute("w:color").set_value("auto")
                                 : shd.append_attribute("w:color").set_value("auto");
        shd.attribute("w:fill") ? shd.attribute("w:fill").set_value(color.c_str())
                                : shd.append_attribute("w:fill").set_value(color.c_str());
        return *this;
    }

    TableCell& TableCell::set_text_direction(const std::string& direction)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node text_dir = tc_pr.child("w:textDirection");
        if (!text_dir) {
            text_dir = tc_pr.append_child("w:textDirection");
        }
        
        text_dir.attribute("w:val") ? text_dir.attribute("w:val").set_value(direction.c_str())
                                    : text_dir.append_attribute("w:val").set_value(direction.c_str());
        return *this;
    }

    TableCell& TableCell::set_margins(double top_pts, double right_pts, double bottom_pts, double left_pts)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_mar = tc_pr.child("w:tcMar");
        if (!tc_mar) {
            tc_mar = tc_pr.append_child("w:tcMar");
        }
        
        struct { const char* name; double value; } margins[] = {
            {"w:top", top_pts}, {"w:right", right_pts}, 
            {"w:bottom", bottom_pts}, {"w:left", left_pts}
        };
        
        for (const auto& margin : margins) {
            pugi::xml_node margin_node = tc_mar.child(margin.name);
            if (!margin_node) {
                margin_node = tc_mar.append_child(margin.name);
            }
            const long long margin_twips = static_cast<long long>(margin.value * 20.0);
            margin_node.attribute("w:w") ? margin_node.attribute("w:w").set_value(std::to_string(margin_twips).c_str())
                                         : margin_node.append_attribute("w:w").set_value(std::to_string(margin_twips).c_str());
            margin_node.attribute("w:type") ? margin_node.attribute("w:type").set_value("dxa")
                                            : margin_node.append_attribute("w:type").set_value("dxa");
        }
        return *this;
    }

    TableCell& TableCell::set_border_style(const std::string& style)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_borders = get_or_create_tc_borders(tc_pr);
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tc_borders.child(border_name);
            if (!border) {
                border = tc_borders.append_child(border_name);
            }
            border.attribute("w:val") ? border.attribute("w:val").set_value(style.c_str())
                                      : border.append_attribute("w:val").set_value(style.c_str());
        }
        return *this;
    }

    TableCell& TableCell::set_border_width(double width_pts)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_borders = get_or_create_tc_borders(tc_pr);
        
        const long long width_eighth_pts = static_cast<long long>(width_pts * 8.0);
        const std::string width_str = std::to_string(width_eighth_pts);
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tc_borders.child(border_name);
            if (!border) {
                border = tc_borders.append_child(border_name);
            }
            border.attribute("w:sz") ? border.attribute("w:sz").set_value(width_str.c_str())
                                     : border.append_attribute("w:sz").set_value(width_str.c_str());
        }
        return *this;
    }

    TableCell& TableCell::set_border_color(const std::string& color)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_borders = get_or_create_tc_borders(tc_pr);
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tc_borders.child(border_name);
            if (!border) {
                border = tc_borders.append_child(border_name);
            }
            border.attribute("w:color") ? border.attribute("w:color").set_value(color.c_str())
                                        : border.append_attribute("w:color").set_value(color.c_str());
        }
        return *this;
    }

    double TableCell::get_width() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto tc_w = tc_pr.child("w:tcW");
            if (tc_w) {
                const auto w_attr = tc_w.attribute("w:w");
                if (w_attr) {
                    return w_attr.as_double() / 20.0;
                }
            }
        }
        return 0.0;
    }

    std::string TableCell::get_width_type() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto tc_w = tc_pr.child("w:tcW");
            if (tc_w) {
                const auto type_attr = tc_w.attribute("w:type");
                if (type_attr) {
                    return type_attr.as_string();
                }
            }
        }
        return "auto";
    }

    std::string TableCell::get_vertical_alignment() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto v_align = tc_pr.child("w:vAlign");
            if (v_align) {
                const auto val_attr = v_align.attribute("w:val");
                if (val_attr) {
                    return val_attr.as_string();
                }
            }
        }
        return "top";
    }

    std::string TableCell::get_background_color() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto shd = tc_pr.child("w:shd");
            if (shd) {
                const auto fill_attr = shd.attribute("w:fill");
                if (fill_attr) {
                    return fill_attr.as_string();
                }
            }
        }
        return "";
    }

    std::string TableCell::get_text_direction() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto text_dir = tc_pr.child("w:textDirection");
            if (text_dir) {
                const auto val_attr = text_dir.attribute("w:val");
                if (val_attr) {
                    return val_attr.as_string();
                }
            }
        }
        return "lrTb";
    }

    Result<std::array<double, 4>> TableCell::get_margins_safe() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto tc_mar = tc_pr.child("w:tcMar");
            if (tc_mar) {
                std::array<double, 4> margins = {0.0, 0.0, 0.0, 0.0}; // top, right, bottom, left
                
                const auto top = tc_mar.child("w:top");
                if (top) {
                    const auto w_attr = top.attribute("w:w");
                    if (w_attr) {
                        margins[0] = w_attr.as_double() / 20.0;
                    }
                }
                const auto right = tc_mar.child("w:right");
                if (right) {
                    const auto w_attr = right.attribute("w:w");
                    if (w_attr) {
                        margins[1] = w_attr.as_double() / 20.0;
                    }
                }
                const auto bottom = tc_mar.child("w:bottom");
                if (bottom) {
                    const auto w_attr = bottom.attribute("w:w");
                    if (w_attr) {
                        margins[2] = w_attr.as_double() / 20.0;
                    }
                }
                const auto left = tc_mar.child("w:left");
                if (left) {
                    const auto w_attr = left.attribute("w:w");
                    if (w_attr) {
                        margins[3] = w_attr.as_double() / 20.0;
                    }
                }
                return Result<std::array<double, 4>>(margins);
            }
        }
        return Result<std::array<double, 4>>(errors::element_not_found("margins", ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
    }

    std::string TableCell::get_border_style() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto tc_borders = tc_pr.child("w:tcBorders");
            if (tc_borders) {
                const auto top_border = tc_borders.child("w:top");
                if (top_border) {
                    const auto val_attr = top_border.attribute("w:val");
                    if (val_attr) {
                        return val_attr.as_string();
                    }
                }
            }
        }
        return "";
    }

    double TableCell::get_border_width() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto tc_borders = tc_pr.child("w:tcBorders");
            if (tc_borders) {
                const auto top_border = tc_borders.child("w:top");
                if (top_border) {
                    const auto sz_attr = top_border.attribute("w:sz");
                    if (sz_attr) {
                        return sz_attr.as_double() / 8.0;
                    }
                }
            }
        }
        return 0.0;
    }

    std::string TableCell::get_border_color() const
    {
        const auto tc_pr = m_currentNode.child("w:tcPr");
        if (tc_pr) {
            const auto tc_borders = tc_pr.child("w:tcBorders");
            if (tc_borders) {
                const auto top_border = tc_borders.child("w:top");
                if (top_border) {
                    const auto color_attr = top_border.attribute("w:color");
                    if (color_attr) {
                        return color_attr.as_string();
                    }
                }
            }
        }
        return "";
    }

    pugi::xml_node TableCell::get_or_create_tc_pr()
    {
        pugi::xml_node tc_pr = m_currentNode.child("w:tcPr");
        if (!tc_pr) {
            tc_pr = m_currentNode.prepend_child("w:tcPr");
        }
        return tc_pr;
    }

    pugi::xml_node TableCell::get_or_create_tc_borders(pugi::xml_node tc_pr)
    {
        pugi::xml_node tc_borders = tc_pr.child("w:tcBorders");
        if (!tc_borders) {
            tc_borders = tc_pr.append_child("w:tcBorders");
        }
        return tc_borders;
    }

    Result<TableCell*> TableCell::set_width_safe(double width_pts)
    {
        if (width_pts < 0) {
            return Result<TableCell*>(errors::invalid_argument("width_pts", "Width must be non-negative",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_value", std::to_string(width_pts))));
        }
        
        if (width_pts > 1000.0) { // Reasonable maximum
            return Result<TableCell*>(errors::validation_failed("width_pts", "Width exceeds maximum allowed value",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("max_width", "1000.0")
                    .with_info("provided_width", std::to_string(width_pts))));
        }
        
        set_width(width_pts);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_width_type_safe(const std::string& type)
    {
        if (type.empty()) {
            return Result<TableCell*>(errors::invalid_argument("type", "Width type cannot be empty"));
        }
        
        if (type != "dxa" && type != "pct" && type != "auto") {
            return Result<TableCell*>(errors::validation_failed("type", "Invalid width type",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_type", type)
                    .with_info("valid_types", "dxa, pct, auto")));
        }
        
        set_width_type(type);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_vertical_alignment_safe(const std::string& alignment)
    {
        if (alignment.empty()) {
            return Result<TableCell*>(errors::invalid_argument("alignment", "Alignment cannot be empty"));
        }
        
        if (alignment != "top" && alignment != "center" && alignment != "bottom") {
            return Result<TableCell*>(errors::validation_failed("alignment", "Invalid vertical alignment",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_alignment", alignment)
                    .with_info("valid_alignments", "top, center, bottom")));
        }
        
        set_vertical_alignment(alignment);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_background_color_safe(const std::string& color)
    {
        if (color.empty()) {
            return Result<TableCell*>(errors::invalid_argument("color", "Color cannot be empty"));
        }
        
        // Basic hex color validation
        if (color.length() != 6 || !std::all_of(color.begin(), color.end(), [](char c) { return std::isxdigit(c); })) {
            return Result<TableCell*>(errors::validation_failed("color", "Invalid hex color format",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_color", color)
                    .with_info("expected_format", "6-digit hex (e.g., FF0000)")));
        }
        
        set_background_color(color);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_text_direction_safe(const std::string& direction)
    {
        if (direction.empty()) {
            return Result<TableCell*>(errors::invalid_argument("direction", "Text direction cannot be empty"));
        }
        
        const std::vector<std::string> valid_directions = {"lrTb", "tbRl", "btLr", "lrTbV", "tbRlV", "tbLrV"};
        if (std::find(valid_directions.begin(), valid_directions.end(), direction) == valid_directions.end()) {
            return Result<TableCell*>(errors::validation_failed("direction", "Invalid text direction",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_direction", direction)
                    .with_info("valid_directions", "lrTb, tbRl, btLr, lrTbV, tbRlV, tbLrV")));
        }
        
        set_text_direction(direction);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_margins_safe(double top_pts, double right_pts, double bottom_pts, double left_pts)
    {
        const double margins[] = {top_pts, right_pts, bottom_pts, left_pts};
        const char* names[] = {"top_pts", "right_pts", "bottom_pts", "left_pts"};
        
        for (int i = 0; i < 4; ++i) {
            if (margins[i] < 0) {
                return Result<TableCell*>(errors::invalid_argument(names[i], "Margin must be non-negative",
                    ErrorContext(__FILE__, __func__, __LINE__)
                        .with_info("provided_value", std::to_string(margins[i]))));
            }
        }
        
        set_margins(top_pts, right_pts, bottom_pts, left_pts);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_border_style_safe(const std::string& style)
    {
        if (style.empty()) {
            return Result<TableCell*>(errors::invalid_argument("style", "Border style cannot be empty"));
        }
        
        const std::vector<std::string> valid_styles = {"single", "double", "dashed", "dotted", "none"};
        if (std::find(valid_styles.begin(), valid_styles.end(), style) == valid_styles.end()) {
            return Result<TableCell*>(errors::validation_failed("style", "Invalid border style",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_style", style)
                    .with_info("valid_styles", "single, double, dashed, dotted, none")));
        }
        
        set_border_style(style);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_border_width_safe(double width_pts)
    {
        if (width_pts < 0) {
            return Result<TableCell*>(errors::invalid_argument("width_pts", "Border width must be non-negative",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_value", std::to_string(width_pts))));
        }
        
        if (width_pts > 20.0) { // Reasonable maximum
            return Result<TableCell*>(errors::validation_failed("width_pts", "Border width exceeds maximum",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("max_width", "20.0")
                    .with_info("provided_width", std::to_string(width_pts))));
        }
        
        set_border_width(width_pts);
        return Result<TableCell*>(this);
    }

    Result<TableCell*> TableCell::set_border_color_safe(const std::string& color)
    {
        if (color.empty()) {
            return Result<TableCell*>(errors::invalid_argument("color", "Border color cannot be empty"));
        }
        
        // Basic hex color validation
        if (color.length() != 6 || !std::all_of(color.begin(), color.end(), [](char c) { return std::isxdigit(c); })) {
            return Result<TableCell*>(errors::validation_failed("color", "Invalid hex color format",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_color", color)
                    .with_info("expected_format", "6-digit hex (e.g., 000000)")));
        }
        
        set_border_color(color);
        return Result<TableCell*>(this);
    }

    // ============================================================================
    // TableRow Result<T> API implementations (Modern, recommended)
    // ============================================================================

    Result<TableRow*> TableRow::set_height_safe(double height_pts)
    {
        if (height_pts < 0) {
            return Result<TableRow*>(errors::invalid_argument("height_pts", "Height must be non-negative",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_value", std::to_string(height_pts))));
        }
        
        if (height_pts > 500.0) { // Reasonable maximum for row height
            return Result<TableRow*>(errors::validation_failed("height_pts", "Height exceeds maximum allowed value",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("max_height", "500.0")
                    .with_info("provided_height", std::to_string(height_pts))));
        }
        
        set_height(height_pts);
        return Result<TableRow*>(this);
    }

    Result<TableRow*> TableRow::set_height_rule_safe(const std::string& rule)
    {
        if (rule.empty()) {
            return Result<TableRow*>(errors::invalid_argument("rule", "Height rule cannot be empty"));
        }
        
        if (rule != "exact" && rule != "atLeast" && rule != "auto") {
            return Result<TableRow*>(errors::validation_failed("rule", "Invalid height rule",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_rule", rule)
                    .with_info("valid_rules", "exact, atLeast, auto")));
        }
        
        set_height_rule(rule);
        return Result<TableRow*>(this);
    }

    Result<TableRow*> TableRow::set_header_row_safe(bool is_header)
    {
        set_header_row(is_header);
        return Result<TableRow*>(this);
    }

    Result<TableRow*> TableRow::set_cant_split_safe(bool cant_split)
    {
        set_cant_split(cant_split);
        return Result<TableRow*>(this);
    }

    // ============================================================================
    // Table Result<T> API implementations (Modern, recommended)
    // ============================================================================

    Result<Table*> Table::set_alignment_safe(const std::string& alignment)
    {
        if (alignment.empty()) {
            return Result<Table*>(errors::invalid_argument("alignment", "Alignment cannot be empty"));
        }
        
        if (alignment != "left" && alignment != "center" && alignment != "right") {
            return Result<Table*>(errors::validation_failed("alignment", "Invalid table alignment",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_alignment", alignment)
                    .with_info("valid_alignments", "left, center, right")));
        }
        
        set_alignment(alignment);
        return Result<Table*>(this);
    }

    Result<Table*> Table::set_width_safe(double width_pts)
    {
        if (width_pts <= 0) {
            return Result<Table*>(errors::invalid_argument("width_pts", "Width must be positive",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_value", std::to_string(width_pts))));
        }
        
        if (width_pts > 2000.0) { // Reasonable maximum for table width
            return Result<Table*>(errors::validation_failed("width_pts", "Width exceeds maximum allowed value",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("max_width", "2000.0")
                    .with_info("provided_width", std::to_string(width_pts))));
        }
        
        set_width(width_pts);
        return Result<Table*>(this);
    }

    Result<Table*> Table::set_border_style_safe(const std::string& style)
    {
        if (style.empty()) {
            return Result<Table*>(errors::invalid_argument("style", "Border style cannot be empty"));
        }
        
        const std::vector<std::string> valid_styles = {"single", "double", "dashed", "dotted", "none"};
        if (std::find(valid_styles.begin(), valid_styles.end(), style) == valid_styles.end()) {
            return Result<Table*>(errors::validation_failed("style", "Invalid border style",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_style", style)
                    .with_info("valid_styles", "single, double, dashed, dotted, none")));
        }
        
        set_border_style(style);
        return Result<Table*>(this);
    }

    Result<Table*> Table::set_border_width_safe(double width_pts)
    {
        if (width_pts < 0) {
            return Result<Table*>(errors::invalid_argument("width_pts", "Border width must be non-negative",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_value", std::to_string(width_pts))));
        }
        
        if (width_pts > 20.0) { // Reasonable maximum
            return Result<Table*>(errors::validation_failed("width_pts", "Border width exceeds maximum",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("max_width", "20.0")
                    .with_info("provided_width", std::to_string(width_pts))));
        }
        
        set_border_width(width_pts);
        return Result<Table*>(this);
    }

    Result<Table*> Table::set_border_color_safe(const std::string& color)
    {
        if (color.empty()) {
            return Result<Table*>(errors::invalid_argument("color", "Border color cannot be empty"));
        }
        
        // Basic hex color validation
        if (color.length() != 6 || !std::all_of(color.begin(), color.end(), [](char c) { return std::isxdigit(c); })) {
            return Result<Table*>(errors::validation_failed("color", "Invalid hex color format",
                ErrorContext(__FILE__, __func__, __LINE__)
                    .with_info("provided_color", color)
                    .with_info("expected_format", "6-digit hex (e.g., 000000)")));
        }
        
        set_border_color(color);
        return Result<Table*>(this);
    }

    Result<Table*> Table::set_cell_margins_safe(double top_pts, double right_pts, double bottom_pts, double left_pts)
    {
        const double margins[] = {top_pts, right_pts, bottom_pts, left_pts};
        const char* names[] = {"top_pts", "right_pts", "bottom_pts", "left_pts"};
        
        for (int i = 0; i < 4; ++i) {
            if (margins[i] < 0) {
                return Result<Table*>(errors::invalid_argument(names[i], "Margin must be non-negative",
                    ErrorContext(__FILE__, __func__, __LINE__)
                        .with_info("provided_value", std::to_string(margins[i]))));
            }
        }
        
        set_cell_margins(top_pts, right_pts, bottom_pts, left_pts);
        return Result<Table*>(this);
    }

    // ============================================================================
    // Table Getter implementations
    // ============================================================================

    std::string Table::get_alignment() const
    {
        const auto tbl_pr = m_currentNode.child("w:tblPr");
        if (tbl_pr) {
            const auto jc = tbl_pr.child("w:jc");
            if (jc) {
                const auto val_attr = jc.attribute("w:val");
                if (val_attr) {
                    return val_attr.as_string();
                }
            }
        }
        return "left";
    }

    double Table::get_width() const
    {
        const auto tbl_pr = m_currentNode.child("w:tblPr");
        if (tbl_pr) {
            const auto tbl_w = tbl_pr.child("w:tblW");
            if (tbl_w) {
                const auto w_attr = tbl_w.attribute("w:w");
                if (w_attr) {
                    return w_attr.as_double() / 20.0; // Convert from twips to points
                }
            }
        }
        return 0.0;
    }

    std::string Table::get_border_style() const
    {
        const auto tbl_pr = m_currentNode.child("w:tblPr");
        if (tbl_pr) {
            const auto tbl_borders = tbl_pr.child("w:tblBorders");
            if (tbl_borders) {
                const auto top_border = tbl_borders.child("w:top");
                if (top_border) {
                    const auto val_attr = top_border.attribute("w:val");
                    if (val_attr) {
                        return val_attr.as_string();
                    }
                }
            }
        }
        return "";
    }

    double Table::get_border_width() const
    {
        const auto tbl_pr = m_currentNode.child("w:tblPr");
        if (tbl_pr) {
            const auto tbl_borders = tbl_pr.child("w:tblBorders");
            if (tbl_borders) {
                const auto top_border = tbl_borders.child("w:top");
                if (top_border) {
                    const auto sz_attr = top_border.attribute("w:sz");
                    if (sz_attr) {
                        return sz_attr.as_double() / 8.0; // Convert from eighths of a point
                    }
                }
            }
        }
        return 0.0;
    }

    std::string Table::get_border_color() const
    {
        const auto tbl_pr = m_currentNode.child("w:tblPr");
        if (tbl_pr) {
            const auto tbl_borders = tbl_pr.child("w:tblBorders");
            if (tbl_borders) {
                const auto top_border = tbl_borders.child("w:top");
                if (top_border) {
                    const auto color_attr = top_border.attribute("w:color");
                    if (color_attr) {
                        return color_attr.as_string();
                    }
                }
            }
        }
        return "";
    }

    Result<std::array<double, 4>> Table::get_cell_margins_safe() const
    {
        const auto tbl_pr = m_currentNode.child("w:tblPr");
        if (tbl_pr) {
            const auto tbl_cell_mar = tbl_pr.child("w:tblCellMar");
            if (tbl_cell_mar) {
                std::array<double, 4> margins = {0.0, 0.0, 0.0, 0.0}; // top, right, bottom, left
                
                const auto top = tbl_cell_mar.child("w:top");
                if (top) {
                    const auto w_attr = top.attribute("w:w");
                    if (w_attr) {
                        margins[0] = w_attr.as_double() / 20.0;
                    }
                }
                const auto right = tbl_cell_mar.child("w:right");
                if (right) {
                    const auto w_attr = right.attribute("w:w");
                    if (w_attr) {
                        margins[1] = w_attr.as_double() / 20.0;
                    }
                }
                const auto bottom = tbl_cell_mar.child("w:bottom");
                if (bottom) {
                    const auto w_attr = bottom.attribute("w:w");
                    if (w_attr) {
                        margins[2] = w_attr.as_double() / 20.0;
                    }
                }
                const auto left = tbl_cell_mar.child("w:left");
                if (left) {
                    const auto w_attr = left.attribute("w:w");
                    if (w_attr) {
                        margins[3] = w_attr.as_double() / 20.0;
                    }
                }
                return Result<std::array<double, 4>>(margins);
            }
        }
        return Result<std::array<double, 4>>(errors::element_not_found("cell_margins", ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
    }

    // ============================================================================
    // Table Setter implementations (Legacy API)
    // ============================================================================

    Table& Table::set_alignment(const std::string& alignment)
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        
        pugi::xml_node jc = tbl_pr.child("w:jc");
        if (!jc) {
            jc = tbl_pr.append_child("w:jc");
        }
        
        jc.remove_attribute("w:val");
        jc.append_attribute("w:val").set_value(alignment.c_str());
        return *this;
    }

    Table& Table::set_width(double width_pts)
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        
        pugi::xml_node tbl_w = tbl_pr.child("w:tblW");
        if (!tbl_w) {
            tbl_w = tbl_pr.append_child("w:tblW");
        }
        
        tbl_w.remove_attribute("w:w");
        tbl_w.remove_attribute("w:type");
        tbl_w.append_attribute("w:w").set_value(static_cast<int>(width_pts * 20)); // Convert to twips
        tbl_w.append_attribute("w:type").set_value("dxa");
        return *this;
    }

    Table& Table::set_border_style(const std::string& style)
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        
        pugi::xml_node tbl_borders = tbl_pr.child("w:tblBorders");
        if (!tbl_borders) {
            tbl_borders = tbl_pr.append_child("w:tblBorders");
        }
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tbl_borders.child(border_name);
            if (!border) {
                border = tbl_borders.append_child(border_name);
            }
            border.remove_attribute("w:val");
            border.append_attribute("w:val").set_value(style.c_str());
        }
        return *this;
    }

    Table& Table::set_border_width(double width_pts)
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        
        pugi::xml_node tbl_borders = tbl_pr.child("w:tblBorders");
        if (!tbl_borders) {
            tbl_borders = tbl_pr.append_child("w:tblBorders");
        }
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tbl_borders.child(border_name);
            if (!border) {
                border = tbl_borders.append_child(border_name);
            }
            border.remove_attribute("w:sz");
            border.append_attribute("w:sz").set_value(static_cast<int>(width_pts * 8)); // Convert to eighths of a point
        }
        return *this;
    }

    Table& Table::set_border_color(const std::string& color)
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        
        pugi::xml_node tbl_borders = tbl_pr.child("w:tblBorders");
        if (!tbl_borders) {
            tbl_borders = tbl_pr.append_child("w:tblBorders");
        }
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tbl_borders.child(border_name);
            if (!border) {
                border = tbl_borders.append_child(border_name);
            }
            border.remove_attribute("w:color");
            border.append_attribute("w:color").set_value(color.c_str());
        }
        return *this;
    }

    Table& Table::set_cell_margins(double top_pts, double right_pts, double bottom_pts, double left_pts)
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        
        pugi::xml_node tbl_cell_mar = tbl_pr.child("w:tblCellMar");
        if (!tbl_cell_mar) {
            tbl_cell_mar = tbl_pr.append_child("w:tblCellMar");
        }
        
        const double margins[] = {top_pts, right_pts, bottom_pts, left_pts};
        const char* margin_names[] = {"w:top", "w:right", "w:bottom", "w:left"};
        
        for (int i = 0; i < 4; ++i) {
            pugi::xml_node margin = tbl_cell_mar.child(margin_names[i]);
            if (!margin) {
                margin = tbl_cell_mar.append_child(margin_names[i]);
            }
            const long long margin_twips = static_cast<long long>(margins[i] * 20.0);
            margin.attribute("w:w") ? margin.attribute("w:w").set_value(std::to_string(margin_twips).c_str())
                                    : margin.append_attribute("w:w").set_value(std::to_string(margin_twips).c_str());
            margin.attribute("w:type") ? margin.attribute("w:type").set_value("dxa")
                                        : margin.append_attribute("w:type").set_value("dxa");
        }
        return *this;
    }

    // ============================================================================
    // TableRow Setter implementations (Legacy API)
    // ============================================================================

    TableRow& TableRow::set_height(double height_pts)
    {
        pugi::xml_node tr_pr = m_currentNode.child("w:trPr");
        if (!tr_pr) {
            tr_pr = m_currentNode.prepend_child("w:trPr");
        }
        
        pugi::xml_node tr_height = tr_pr.child("w:trHeight");
        if (!tr_height) {
            tr_height = tr_pr.append_child("w:trHeight");
        }
        
        tr_height.remove_attribute("w:val");
        tr_height.append_attribute("w:val").set_value(static_cast<int>(height_pts * 20)); // Convert to twips
        return *this;
    }

    TableRow& TableRow::set_height_rule(const std::string& rule)
    {
        pugi::xml_node tr_pr = m_currentNode.child("w:trPr");
        if (!tr_pr) {
            tr_pr = m_currentNode.prepend_child("w:trPr");
        }
        
        pugi::xml_node tr_height = tr_pr.child("w:trHeight");
        if (!tr_height) {
            tr_height = tr_pr.append_child("w:trHeight");
        }
        
        tr_height.remove_attribute("w:hRule");
        tr_height.append_attribute("w:hRule").set_value(rule.c_str());
        return *this;
    }

    TableRow& TableRow::set_header_row(bool is_header)
    {
        pugi::xml_node tr_pr = m_currentNode.child("w:trPr");
        if (!tr_pr) {
            tr_pr = m_currentNode.prepend_child("w:trPr");
        }
        
        pugi::xml_node tbl_header = tr_pr.child("w:tblHeader");
        if (is_header) {
            if (!tbl_header) {
                tbl_header = tr_pr.append_child("w:tblHeader");
            }
        } else {
            if (tbl_header) {
                tr_pr.remove_child(tbl_header);
            }
        }
        return *this;
    }

    TableRow& TableRow::set_cant_split(bool cant_split)
    {
        pugi::xml_node tr_pr = m_currentNode.child("w:trPr");
        if (!tr_pr) {
            tr_pr = m_currentNode.prepend_child("w:trPr");
        }
        
        pugi::xml_node cant_split_node = tr_pr.child("w:cantSplit");
        if (cant_split) {
            if (!cant_split_node) {
                cant_split_node = tr_pr.append_child("w:cantSplit");
            }
        } else {
            if (cant_split_node) {
                tr_pr.remove_child(cant_split_node);
            }
        }
        return *this;
    }

    // ============================================================================
    // TableRow Getter implementations
    // ============================================================================

    double TableRow::get_height() const
    {
        const auto tr_pr = m_currentNode.child("w:trPr");
        if (tr_pr) {
            const auto tr_height = tr_pr.child("w:trHeight");
            if (tr_height) {
                const auto val_attr = tr_height.attribute("w:val");
                if (val_attr) {
                    return val_attr.as_double() / 20.0; // Convert from twips to points
                }
            }
        }
        return 0.0;
    }

    std::string TableRow::get_height_rule() const
    {
        const auto tr_pr = m_currentNode.child("w:trPr");
        if (tr_pr) {
            const auto tr_height = tr_pr.child("w:trHeight");
            if (tr_height) {
                const auto rule_attr = tr_height.attribute("w:hRule");
                if (rule_attr) {
                    return rule_attr.as_string();
                }
            }
        }
        return "auto";
    }

    bool TableRow::is_header_row() const
    {
        const auto tr_pr = m_currentNode.child("w:trPr");
        if (tr_pr) {
            const auto tbl_header = tr_pr.child("w:tblHeader");
            return tbl_header != nullptr;
        }
        return false;
    }

    bool TableRow::get_cant_split() const
    {
        const auto tr_pr = m_currentNode.child("w:trPr");
        if (tr_pr) {
            const auto cant_split_node = tr_pr.child("w:cantSplit");
            return cant_split_node != nullptr;
        }
        return false;
    }

    // ============================================================================
    // Table Basic implementations (Navigation and Element Management)
    // ============================================================================

    Table::Table(pugi::xml_node parent, pugi::xml_node current) : DocxElement(parent, current)
    {
        m_tableRow.set_parent(m_currentNode);
        m_tableRow.set_current(m_currentNode.child("w:tr"));
    }

    void Table::set_parent(pugi::xml_node node)
    {
        m_parentNode = node;
    }

    void Table::set_current(pugi::xml_node node)
    {
        m_currentNode = node;
        m_tableRow.set_parent(m_currentNode);
        m_tableRow.set_current(m_currentNode.child("w:tr"));
    }

    bool Table::has_next() const
    {
        return m_currentNode.next_sibling("w:tbl") != nullptr;
    }

    bool Table::has_next_same_type() const
    {
        return m_currentNode.next_sibling("w:tbl") != nullptr;
    }

    Table& Table::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:tbl");
        m_tableRow.set_parent(m_currentNode);
        m_tableRow.set_current(m_currentNode.child("w:tr"));
        return *this;
    }

    bool Table::try_advance()
    {
        pugi::xml_node next = m_currentNode.next_sibling("w:tbl");
        if (next) {
            m_currentNode = next;
            m_tableRow.set_parent(m_currentNode);
            m_tableRow.set_current(m_currentNode.child("w:tr"));
            return true;
        }
        return false;
    }

    bool Table::can_advance() const
    {
        return has_next();
    }

    bool Table::move_to_next_table()
    {
        return try_advance();
    }

    absl::enable_if_t<is_docx_element<TableRow>::value, ElementRange<TableRow>> Table::rows()
    {
        TableRow first_row(m_currentNode, m_currentNode.child("w:tr"));
        return ElementRange<TableRow>(first_row);
    }

    TableRow& Table::add_row()
    {
        pugi::xml_node new_row = m_currentNode.append_child("w:tr");
        
        // Add a default cell to the new row
        pugi::xml_node new_cell = new_row.append_child("w:tc");
        pugi::xml_node new_p = new_cell.append_child("w:p");
        
        // Update the TableRow to point to the new row
        m_tableRow.set_current(new_row);
        return m_tableRow;
    }

    TableRow& Table::get_row(int index)
    {
        pugi::xml_node row = m_currentNode.child("w:tr");
        for (int i = 0; i < index && row; ++i) {
            row = row.next_sibling("w:tr");
        }
        
        if (row) {
            m_tableRow.set_current(row);
        }
        return m_tableRow;
    }

    int Table::row_count() const
    {
        int count = 0;
        for (pugi::xml_node row = m_currentNode.child("w:tr"); row; row = row.next_sibling("w:tr")) {
            ++count;
        }
        return count;
    }

    // ============================================================================
    // TableRow Basic implementations (Navigation and Element Management)
    // ============================================================================

    TableRow::TableRow(pugi::xml_node parent, pugi::xml_node current) : DocxElement(parent, current)
    {
        m_tableCell.set_parent(m_currentNode);
        m_tableCell.set_current(m_currentNode.child("w:tc"));
    }

    void TableRow::set_parent(pugi::xml_node node)
    {
        m_parentNode = node;
    }

    void TableRow::set_current(pugi::xml_node node)
    {
        m_currentNode = node;
        m_tableCell.set_parent(m_currentNode);
        m_tableCell.set_current(m_currentNode.child("w:tc"));
    }

    bool TableRow::has_next() const
    {
        return m_currentNode.next_sibling("w:tr") != nullptr;
    }

    bool TableRow::has_next_same_type() const
    {
        return m_currentNode.next_sibling("w:tr") != nullptr;
    }

    TableRow& TableRow::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:tr");
        m_tableCell.set_parent(m_currentNode);
        m_tableCell.set_current(m_currentNode.child("w:tc"));
        return *this;
    }

    bool TableRow::try_advance()
    {
        pugi::xml_node next = m_currentNode.next_sibling("w:tr");
        if (next) {
            m_currentNode = next;
            m_tableCell.set_parent(m_currentNode);
            m_tableCell.set_current(m_currentNode.child("w:tc"));
            return true;
        }
        return false;
    }

    bool TableRow::can_advance() const
    {
        return has_next();
    }

    bool TableRow::move_to_next_row()
    {
        return try_advance();
    }

    absl::enable_if_t<is_docx_element<TableCell>::value, ElementRange<TableCell>> TableRow::cells()
    {
        TableCell first_cell(m_currentNode, m_currentNode.child("w:tc"));
        return ElementRange<TableCell>(first_cell);
    }

    TableCell& TableRow::add_cell()
    {
        pugi::xml_node new_cell = m_currentNode.append_child("w:tc");
        pugi::xml_node new_p = new_cell.append_child("w:p");
        
        // Update the TableCell to point to the new cell
        m_tableCell.set_current(new_cell);
        return m_tableCell;
    }

    TableCell& TableRow::get_cell(int index)
    {
        pugi::xml_node cell = m_currentNode.child("w:tc");
        for (int i = 0; i < index && cell; ++i) {
            cell = cell.next_sibling("w:tc");
        }
        
        if (cell) {
            m_tableCell.set_current(cell);
        }
        return m_tableCell;
    }

    int TableRow::cell_count() const
    {
        int count = 0;
        for (pugi::xml_node cell = m_currentNode.child("w:tc"); cell; cell = cell.next_sibling("w:tc")) {
            ++count;
        }
        return count;
    }

} // namespace duckx
