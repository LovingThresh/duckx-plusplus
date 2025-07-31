/*!
 * @file BaseElement_Run.cpp
 * @brief Implementation of Run class for text runs within paragraphs
 * 
 * Contains implementations for text formatting, style application,
 * and text manipulation functionality.
 */
#include "BaseElement_Run.hpp"

#include <cctype>
#include <map>
#include <cmath>

#include "StyleManager.hpp"

namespace duckx
{
    /*! @brief Convert highlight color enum to string */
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

    // Modern Result<T> API for style application (recommended)
    Result<void> Run::apply_style_safe(const StyleManager& style_manager, const std::string& style_name)
    {
        // Verify style exists and is compatible
        auto style_result = style_manager.get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        const Style* style = style_result.value();
        if (style->type() != StyleType::CHARACTER && style->type() != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Style '%s' is not a character or mixed style", style_name),
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        // Get or create run properties node
        pugi::xml_node rpr = get_or_create_rPr();
        if (!rpr) {
            return Result<void>(errors::xml_manipulation_failed(
                "Failed to create run properties node",
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        // Apply style reference
        pugi::xml_node style_ref = rpr.child("w:rStyle");
        if (!style_ref) {
            style_ref = rpr.append_child("w:rStyle");
        }
        style_ref.attribute("w:val") ? 
            style_ref.attribute("w:val").set_value(style_name.c_str()) :
            style_ref.append_attribute("w:val").set_value(style_name.c_str());
        
        return Result<void>{};
    }

    Result<std::string> Run::get_style_safe() const
    {
        pugi::xml_node rpr = m_currentNode.child("w:rPr");
        if (!rpr) {
            return Result<std::string>{std::string{}}; // No style applied, return empty string
        }
        
        pugi::xml_node style_ref = rpr.child("w:rStyle");
        if (!style_ref) {
            return Result<std::string>{std::string{}}; // No style applied
        }
        
        pugi::xml_attribute val_attr = style_ref.attribute("w:val");
        if (!val_attr) {
            return Result<std::string>{std::string{}}; // No style value
        }
        
        return Result<std::string>{std::string(val_attr.value())};
    }

    Result<void> Run::remove_style_safe()
    {
        pugi::xml_node rpr = m_currentNode.child("w:rPr");
        if (!rpr) {
            return Result<void>{}; // No properties to remove
        }
        
        pugi::xml_node style_ref = rpr.child("w:rStyle");
        if (style_ref) {
            rpr.remove_child(style_ref);
        }
        
        return Result<void>{};
    }

} // namespace duckx