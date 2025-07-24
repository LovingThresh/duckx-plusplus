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
        if (const auto pPr = m_currentNode.child("w:pPr"))
        {
            if (const auto jc = pPr.child("w:jc"))
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
        if (const auto pPr = m_currentNode.child("w:pPr"))
        {
            if (const auto spacingNode = pPr.child("w:spacing"))
            {
                if (const auto line = spacingNode.attribute("w:line"))
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
        if (const auto pPr = m_currentNode.child("w:pPr"))
        {
            if (const auto spacingNode = pPr.child("w:spacing"))
            {
                bool found = false;
                if (const auto before = spacingNode.attribute("w:before"))
                {
                    before_pts = before.as_double() / 20.0;
                    found = true;
                }
                if (const auto after = spacingNode.attribute("w:after"))
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
        if (const auto pPr = m_currentNode.child("w:pPr"))
        {
            if (const auto indNode = pPr.child("w:ind"))
            {
                bool found = false;
                if (const auto left = indNode.attribute("w:left"))
                {
                    left_pts = left.as_double() / 20.0;
                    found = true;
                }
                if (const auto right = indNode.attribute("w:right"))
                {
                    right_pts = right.as_double() / 20.0;
                    found = true;
                }
                if (const auto firstLine = indNode.attribute("w:firstLine"))
                {
                    first_line_pts = firstLine.as_double() / 20.0;
                    found = true;
                }
                if (const auto hanging = indNode.attribute("w:hanging"))
                {
                    // A hanging indent is a negative first line indent
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
        if (const auto pPr = m_currentNode.child("w:pPr"))
        {
            if (const auto numPr = pPr.child("w:numPr"))
            {
                if (const auto ilvl = numPr.child("w:ilvl"))
                {
                    level = ilvl.attribute("w:val").as_int(-1);
                }
                else
                {
                    level = -1;
                }

                if (const auto id = numPr.child("w:numId"))
                {
                    numId = id.attribute("w:val").as_int(-1);
                }
                else
                {
                    numId = -1;
                }

                // As before, we can't easily determine the type (BULLET/NUMBER)
                // from this XML alone. We'll default to BULLET as a placeholder.
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

        if (*text != 0 && (isspace(text[0]) || isspace(text[strlen(text) - 1])))
            new_run_text.append_attribute("xml:space").set_value("preserve");
        new_run_text.text().set(text);

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

    TableCell& TableCell::set_margins(double top_pts, double bottom_pts, double left_pts, double right_pts)
    {
        pugi::xml_node tc_pr = get_or_create_tc_pr();
        pugi::xml_node tc_mar = tc_pr.child("w:tcMar");
        if (!tc_mar) {
            tc_mar = tc_pr.append_child("w:tcMar");
        }
        
        struct { const char* name; double value; } margins[] = {
            {"w:top", top_pts}, {"w:bottom", bottom_pts}, 
            {"w:left", left_pts}, {"w:right", right_pts}
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

    bool TableCell::get_width(double& width_pts) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto tc_w = tc_pr.child("w:tcW")) {
                if (const auto w_attr = tc_w.attribute("w:w")) {
                    width_pts = w_attr.as_double() / 20.0;
                    return true;
                }
            }
        }
        return false;
    }

    bool TableCell::get_width_type(std::string& type) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto tc_w = tc_pr.child("w:tcW")) {
                if (const auto type_attr = tc_w.attribute("w:type")) {
                    type = type_attr.as_string();
                    return true;
                }
            }
        }
        return false;
    }

    bool TableCell::get_vertical_alignment(std::string& align) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto v_align = tc_pr.child("w:vAlign")) {
                if (const auto val_attr = v_align.attribute("w:val")) {
                    align = val_attr.as_string();
                    return true;
                }
            }
        }
        return false;
    }

    bool TableCell::get_background_color(std::string& color) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto shd = tc_pr.child("w:shd")) {
                if (const auto fill_attr = shd.attribute("w:fill")) {
                    color = fill_attr.as_string();
                    return true;
                }
            }
        }
        return false;
    }

    bool TableCell::get_text_direction(std::string& direction) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto text_dir = tc_pr.child("w:textDirection")) {
                if (const auto val_attr = text_dir.attribute("w:val")) {
                    direction = val_attr.as_string();
                    return true;
                }
            }
        }
        return false;
    }

    bool TableCell::get_margins(double& top_pts, double& bottom_pts, double& left_pts, double& right_pts) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto tc_mar = tc_pr.child("w:tcMar")) {
                bool found = false;
                if (const auto top = tc_mar.child("w:top")) {
                    if (const auto w_attr = top.attribute("w:w")) {
                        top_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                if (const auto bottom = tc_mar.child("w:bottom")) {
                    if (const auto w_attr = bottom.attribute("w:w")) {
                        bottom_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                if (const auto left = tc_mar.child("w:left")) {
                    if (const auto w_attr = left.attribute("w:w")) {
                        left_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                if (const auto right = tc_mar.child("w:right")) {
                    if (const auto w_attr = right.attribute("w:w")) {
                        right_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                return found;
            }
        }
        return false;
    }

    bool TableCell::get_border_style(std::string& style) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto tc_borders = tc_pr.child("w:tcBorders")) {
                if (const auto top_border = tc_borders.child("w:top")) {
                    if (const auto val_attr = top_border.attribute("w:val")) {
                        style = val_attr.as_string();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool TableCell::get_border_width(double& width_pts) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto tc_borders = tc_pr.child("w:tcBorders")) {
                if (const auto top_border = tc_borders.child("w:top")) {
                    if (const auto sz_attr = top_border.attribute("w:sz")) {
                        width_pts = sz_attr.as_double() / 8.0;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool TableCell::get_border_color(std::string& color) const
    {
        if (const auto tc_pr = m_currentNode.child("w:tcPr")) {
            if (const auto tc_borders = tc_pr.child("w:tcBorders")) {
                if (const auto top_border = tc_borders.child("w:top")) {
                    if (const auto color_attr = top_border.attribute("w:color")) {
                        color = color_attr.as_string();
                        return true;
                    }
                }
            }
        }
        return false;
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

    TableRow::TableRow(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

    void TableRow::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tr");

        m_tableCell.set_parent(m_currentNode);
    }

    void TableRow::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    bool TableRow::has_next() const
    {
        if (!m_currentNode) return false;
        return find_next_sibling("w:tr") != nullptr;
    }

    bool TableRow::has_next_same_type() const
    {
        if (!m_currentNode)
            return false;

        return !find_next_sibling("w:tr").empty();
    }

    absl::enable_if_t<is_docx_element<TableCell>::value, ElementRange<TableCell>> TableRow::cells()
    {
        if (m_currentNode)
        {
            m_tableCell.set_current(m_currentNode.child("w:tc"));
        }
        else
        {
            m_tableCell.set_current(pugi::xml_node());
        }

        m_tableCell.set_parent(m_currentNode);

        return make_element_range(m_tableCell);
    }

    TableRow& TableRow::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:tr");
        return *this;
    }

    bool TableRow::try_advance()
    {
        const pugi::xml_node next_row = find_next_sibling("w:tr");
        if (!next_row.empty())
        {
            m_currentNode = next_row;
            return true;
        }
        return false;
    }

    bool TableRow::can_advance() const
    {
        return !find_next_sibling("w:tr").empty();
    }

    bool TableRow::move_to_next_row()
    {
        m_currentNode = find_next_sibling("w:tr");
        m_tableCell.set_parent(m_currentNode);
        return true;
    }

    TableRow& TableRow::set_height(double height_pts)
    {
        pugi::xml_node tr_pr = get_or_create_tr_pr();
        pugi::xml_node tr_height = tr_pr.child("w:trHeight");
        if (!tr_height) {
            tr_height = tr_pr.append_child("w:trHeight");
        }
        
        const long long height_twips = static_cast<long long>(height_pts * 20.0);
        tr_height.attribute("w:val") ? tr_height.attribute("w:val").set_value(std::to_string(height_twips).c_str())
                                     : tr_height.append_attribute("w:val").set_value(std::to_string(height_twips).c_str());
        return *this;
    }

    TableRow& TableRow::set_height_rule(const std::string& rule)
    {
        pugi::xml_node tr_pr = get_or_create_tr_pr();
        pugi::xml_node tr_height = tr_pr.child("w:trHeight");
        if (!tr_height) {
            tr_height = tr_pr.append_child("w:trHeight");
        }
        
        tr_height.attribute("w:hRule") ? tr_height.attribute("w:hRule").set_value(rule.c_str())
                                       : tr_height.append_attribute("w:hRule").set_value(rule.c_str());
        return *this;
    }

    TableRow& TableRow::set_header_row(bool is_header)
    {
        pugi::xml_node tr_pr = get_or_create_tr_pr();
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
        pugi::xml_node tr_pr = get_or_create_tr_pr();
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

    bool TableRow::get_height(double& height_pts) const
    {
        if (const auto tr_pr = m_currentNode.child("w:trPr")) {
            if (const auto tr_height = tr_pr.child("w:trHeight")) {
                if (const auto val_attr = tr_height.attribute("w:val")) {
                    height_pts = val_attr.as_double() / 20.0;
                    return true;
                }
            }
        }
        return false;
    }

    bool TableRow::get_height_rule(std::string& rule) const
    {
        if (const auto tr_pr = m_currentNode.child("w:trPr")) {
            if (const auto tr_height = tr_pr.child("w:trHeight")) {
                if (const auto rule_attr = tr_height.attribute("w:hRule")) {
                    rule = rule_attr.as_string();
                    return true;
                }
            }
        }
        return false;
    }

    bool TableRow::is_header_row() const
    {
        if (const auto tr_pr = m_currentNode.child("w:trPr")) {
            return tr_pr.child("w:tblHeader") != nullptr;
        }
        return false;
    }

    bool TableRow::get_cant_split() const
    {
        if (const auto tr_pr = m_currentNode.child("w:trPr")) {
            return tr_pr.child("w:cantSplit") != nullptr;
        }
        return false;
    }

    pugi::xml_node TableRow::get_or_create_tr_pr()
    {
        pugi::xml_node tr_pr = m_currentNode.child("w:trPr");
        if (!tr_pr) {
            tr_pr = m_currentNode.prepend_child("w:trPr");
        }
        return tr_pr;
    }

    Table::Table(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

    void Table::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tbl");

        m_tableRow.set_parent(m_currentNode);
    }

    void Table::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    bool Table::has_next() const
    {
        if (!m_currentNode) return false;
        return find_next_sibling("w:tbl") != nullptr;
    }

    bool Table::has_next_same_type() const
    {
        if (!m_currentNode)
            return false;

        return !find_next_sibling("w:tbl").empty();
    }

    absl::enable_if_t<is_docx_element<TableRow>::value, ElementRange<TableRow>> Table::rows()
    {
        if (m_currentNode)
        {
            m_tableRow.set_current(m_currentNode.child("w:tr"));
        }
        else
        {
            m_tableRow.set_current(pugi::xml_node());
        }

        m_tableRow.set_parent(m_currentNode);

        return make_element_range(m_tableRow);
    }

    Table& Table::advance()
    {
        m_currentNode = m_currentNode.next_sibling("w:tbl");
        m_tableRow.set_parent(m_currentNode);
        return *this;
    }

    bool Table::try_advance()
    {
        const pugi::xml_node next_table = find_next_sibling("w:tbl");
        if (!next_table.empty())
        {
            m_currentNode = next_table;
            m_tableRow.set_parent(m_currentNode);
            return true;
        }
        return false;
    }

    bool Table::can_advance() const
    {
        return !find_next_sibling("w:tbl").empty();
    }

    bool Table::move_to_next_table()
    {
        m_currentNode = find_next_sibling("w:tbl");
        m_tableRow.set_parent(m_currentNode);
        return true;
    }

    Table& Table::set_alignment(Alignment align)
    {
        pugi::xml_node tbl_pr = get_or_create_tbl_pr();
        pugi::xml_node jc_node = tbl_pr.child("w:jc");
        if (!jc_node) {
            jc_node = tbl_pr.append_child("w:jc");
        }
        
        std::string align_val;
        switch (align) {
            case Alignment::LEFT: align_val = "left"; break;
            case Alignment::CENTER: align_val = "center"; break;
            case Alignment::RIGHT: align_val = "right"; break;
            case Alignment::BOTH: align_val = "both"; break;
        }
        jc_node.attribute("w:val") ? jc_node.attribute("w:val").set_value(align_val.c_str())
                                   : jc_node.append_attribute("w:val").set_value(align_val.c_str());
        return *this;
    }

    Table& Table::set_width(double width_pts)
    {
        pugi::xml_node tbl_pr = get_or_create_tbl_pr();
        pugi::xml_node tbl_w_node = tbl_pr.child("w:tblW");
        if (!tbl_w_node) {
            tbl_w_node = tbl_pr.append_child("w:tblW");
        }
        
        const long long width_twips = static_cast<long long>(width_pts * 20.0);
        tbl_w_node.attribute("w:w") ? tbl_w_node.attribute("w:w").set_value(std::to_string(width_twips).c_str())
                                    : tbl_w_node.append_attribute("w:w").set_value(std::to_string(width_twips).c_str());
        tbl_w_node.attribute("w:type") ? tbl_w_node.attribute("w:type").set_value("dxa")
                                       : tbl_w_node.append_attribute("w:type").set_value("dxa");
        return *this;
    }

    Table& Table::set_border_style(const std::string& style)
    {
        pugi::xml_node tbl_pr = get_or_create_tbl_pr();
        pugi::xml_node tbl_borders = get_or_create_tbl_borders(tbl_pr);
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right", "w:insideH", "w:insideV"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tbl_borders.child(border_name);
            if (!border) {
                border = tbl_borders.append_child(border_name);
            }
            border.attribute("w:val") ? border.attribute("w:val").set_value(style.c_str())
                                      : border.append_attribute("w:val").set_value(style.c_str());
        }
        return *this;
    }

    Table& Table::set_border_width(double width_pts)
    {
        pugi::xml_node tbl_pr = get_or_create_tbl_pr();
        pugi::xml_node tbl_borders = get_or_create_tbl_borders(tbl_pr);
        
        const long long width_eighth_pts = static_cast<long long>(width_pts * 8.0);
        const std::string width_str = std::to_string(width_eighth_pts);
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right", "w:insideH", "w:insideV"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tbl_borders.child(border_name);
            if (!border) {
                border = tbl_borders.append_child(border_name);
            }
            border.attribute("w:sz") ? border.attribute("w:sz").set_value(width_str.c_str())
                                     : border.append_attribute("w:sz").set_value(width_str.c_str());
        }
        return *this;
    }

    Table& Table::set_border_color(const std::string& color)
    {
        pugi::xml_node tbl_pr = get_or_create_tbl_pr();
        pugi::xml_node tbl_borders = get_or_create_tbl_borders(tbl_pr);
        
        const char* border_names[] = {"w:top", "w:left", "w:bottom", "w:right", "w:insideH", "w:insideV"};
        for (const char* border_name : border_names) {
            pugi::xml_node border = tbl_borders.child(border_name);
            if (!border) {
                border = tbl_borders.append_child(border_name);
            }
            border.attribute("w:color") ? border.attribute("w:color").set_value(color.c_str())
                                        : border.append_attribute("w:color").set_value(color.c_str());
        }
        return *this;
    }

    Table& Table::set_cell_margins(double top_pts, double bottom_pts, double left_pts, double right_pts)
    {
        pugi::xml_node tbl_pr = get_or_create_tbl_pr();
        pugi::xml_node tbl_cell_mar = tbl_pr.child("w:tblCellMar");
        if (!tbl_cell_mar) {
            tbl_cell_mar = tbl_pr.append_child("w:tblCellMar");
        }
        
        struct { const char* name; double value; } margins[] = {
            {"w:top", top_pts}, {"w:bottom", bottom_pts}, 
            {"w:left", left_pts}, {"w:right", right_pts}
        };
        
        for (const auto& margin : margins) {
            pugi::xml_node margin_node = tbl_cell_mar.child(margin.name);
            if (!margin_node) {
                margin_node = tbl_cell_mar.append_child(margin.name);
            }
            const long long margin_twips = static_cast<long long>(margin.value * 20.0);
            margin_node.attribute("w:w") ? margin_node.attribute("w:w").set_value(std::to_string(margin_twips).c_str())
                                         : margin_node.append_attribute("w:w").set_value(std::to_string(margin_twips).c_str());
            margin_node.attribute("w:type") ? margin_node.attribute("w:type").set_value("dxa")
                                            : margin_node.append_attribute("w:type").set_value("dxa");
        }
        return *this;
    }

    Alignment Table::get_alignment() const
    {
        if (const auto tbl_pr = m_currentNode.child("w:tblPr")) {
            if (const auto jc = tbl_pr.child("w:jc")) {
                const std::string val = jc.attribute("w:val").as_string();
                if (val == "center") return Alignment::CENTER;
                if (val == "right") return Alignment::RIGHT;
                if (val == "both") return Alignment::BOTH;
            }
        }
        return Alignment::LEFT;
    }

    bool Table::get_width(double& width_pts) const
    {
        if (const auto tbl_pr = m_currentNode.child("w:tblPr")) {
            if (const auto tbl_w = tbl_pr.child("w:tblW")) {
                if (const auto w_attr = tbl_w.attribute("w:w")) {
                    width_pts = w_attr.as_double() / 20.0;
                    return true;
                }
            }
        }
        return false;
    }

    bool Table::get_border_style(std::string& style) const
    {
        if (const auto tbl_pr = m_currentNode.child("w:tblPr")) {
            if (const auto tbl_borders = tbl_pr.child("w:tblBorders")) {
                if (const auto top_border = tbl_borders.child("w:top")) {
                    if (const auto val_attr = top_border.attribute("w:val")) {
                        style = val_attr.as_string();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Table::get_border_width(double& width_pts) const
    {
        if (const auto tbl_pr = m_currentNode.child("w:tblPr")) {
            if (const auto tbl_borders = tbl_pr.child("w:tblBorders")) {
                if (const auto top_border = tbl_borders.child("w:top")) {
                    if (const auto sz_attr = top_border.attribute("w:sz")) {
                        width_pts = sz_attr.as_double() / 8.0;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Table::get_border_color(std::string& color) const
    {
        if (const auto tbl_pr = m_currentNode.child("w:tblPr")) {
            if (const auto tbl_borders = tbl_pr.child("w:tblBorders")) {
                if (const auto top_border = tbl_borders.child("w:top")) {
                    if (const auto color_attr = top_border.attribute("w:color")) {
                        color = color_attr.as_string();
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool Table::get_cell_margins(double& top_pts, double& bottom_pts, double& left_pts, double& right_pts) const
    {
        if (const auto tbl_pr = m_currentNode.child("w:tblPr")) {
            if (const auto tbl_cell_mar = tbl_pr.child("w:tblCellMar")) {
                bool found = false;
                if (const auto top = tbl_cell_mar.child("w:top")) {
                    if (const auto w_attr = top.attribute("w:w")) {
                        top_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                if (const auto bottom = tbl_cell_mar.child("w:bottom")) {
                    if (const auto w_attr = bottom.attribute("w:w")) {
                        bottom_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                if (const auto left = tbl_cell_mar.child("w:left")) {
                    if (const auto w_attr = left.attribute("w:w")) {
                        left_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                if (const auto right = tbl_cell_mar.child("w:right")) {
                    if (const auto w_attr = right.attribute("w:w")) {
                        right_pts = w_attr.as_double() / 20.0;
                        found = true;
                    }
                }
                return found;
            }
        }
        return false;
    }

    pugi::xml_node Table::get_or_create_tbl_pr()
    {
        pugi::xml_node tbl_pr = m_currentNode.child("w:tblPr");
        if (!tbl_pr) {
            tbl_pr = m_currentNode.prepend_child("w:tblPr");
        }
        return tbl_pr;
    }

    pugi::xml_node Table::get_or_create_tbl_borders(pugi::xml_node tbl_pr)
    {
        pugi::xml_node tbl_borders = tbl_pr.child("w:tblBorders");
        if (!tbl_borders) {
            tbl_borders = tbl_pr.append_child("w:tblBorders");
        }
        return tbl_borders;
    }
} // namespace duckx
