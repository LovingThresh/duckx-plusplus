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
#include <zip.h>

#include "Document.hpp"
#include "HyperlinkManager.hpp"

namespace duckx
{
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

    void Run::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:r");
    }

    void Run::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    pugi::xml_node DocxElement::getNode() const
    {
        return m_currentNode;
    }

    pugi::xml_node DocxElement::findNextSibling(const std::string& name) const
    {
        const pugi::xml_node sibling = m_currentNode.next_sibling(name.c_str());
        return sibling;
    }

    Run::Run(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

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

    Run& Run::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        return *this;
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

    bool Run::has_next() const
    {
        return m_currentNode != nullptr;
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
        return m_currentNode != nullptr;
    }

    TableCell& TableCell::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        return *this;
    }

    ElementRange<Paragraph> TableCell::paragraphs()
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

        return ElementRange<Paragraph>(m_paragraph);
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

    TableRow& TableRow::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        return *this;
    }

    ElementRange<TableCell> TableRow::cells()
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

        return ElementRange<TableCell>(m_tableCell);
    }

    bool TableRow::has_next() const
    {
        return m_currentNode != nullptr;
    }

    Table::Table(const pugi::xml_node parent, const pugi::xml_node current)
        : DocxElement(parent, current) {}

    void Table::set_parent(const pugi::xml_node node)
    {
        m_parentNode = node;
        m_currentNode = m_parentNode.child("w:tbl");

        m_tableRow.set_parent(m_currentNode);
    }

    bool Table::has_next() const
    {
        return m_currentNode != nullptr;
    }

    Table& Table::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        m_tableRow.set_parent(m_currentNode);
        return *this;
    }

    void Table::set_current(const pugi::xml_node node)
    {
        m_currentNode = node;
    }

    ElementRange<TableRow> Table::rows()
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

        return ElementRange<TableRow>(m_tableRow);
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

    Paragraph& Paragraph::next()
    {
        m_currentNode = m_currentNode.next_sibling();
        m_run.set_parent(m_currentNode);
        return *this;
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

    pugi::xml_node Paragraph::get_or_create_pPr()
    {
        pugi::xml_node pPr_node = m_currentNode.child("w:pPr");
        if (!pPr_node)
        {
            pPr_node = m_currentNode.insert_child_before("w:pPr", m_currentNode.first_child());
        }
        return pPr_node;
    }

    bool Paragraph::has_next() const
    {
        return m_currentNode != nullptr;
    }

    ElementRange<Run> Paragraph::runs()
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

        return ElementRange<Run>(m_run);
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
} // namespace duckx
