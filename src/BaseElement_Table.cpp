/*!
 * @file BaseElement_Table.cpp
 * @brief Implementation of table-related classes (Table, TableRow, TableCell)
 * 
 * Contains implementations for table structure, formatting, borders,
 * margins, and Result<T> API for all table elements.
 */
#include "BaseElement_Table.hpp"

#include <algorithm>
#include <vector>

#include "StyleManager.hpp"

namespace duckx
{
    // ============================================================================
    // TableCell implementations
    // ============================================================================

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

    // Legacy API implementations
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

    // Getters
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

    // Helper methods
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

    // Modern Result<T> API implementations
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
    // TableRow implementations 
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

    // TableRow getters
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

    // TableRow helper method
    pugi::xml_node TableRow::get_or_create_tr_pr()
    {
        pugi::xml_node tr_pr = m_currentNode.child("w:trPr");
        if (!tr_pr) {
            tr_pr = m_currentNode.prepend_child("w:trPr");
        }
        return tr_pr;
    }

    // TableRow legacy setters
    TableRow& TableRow::set_height(double height_pts)
    {
        pugi::xml_node tr_pr = get_or_create_tr_pr();
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
        pugi::xml_node tr_pr = get_or_create_tr_pr();
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

    // TableRow Result<T> API
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
    // Table implementations
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

    // Table getters
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

    // Table legacy setters
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

    // Table Result<T> API
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

    // Table Style Application Methods
    Result<void> Table::apply_style_safe(const StyleManager& style_manager, const std::string& style_name)
    {
        // Verify style exists and is compatible
        auto style_result = style_manager.get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        const Style* style = style_result.value();
        if (style->type() != StyleType::TABLE && style->type() != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Style '%s' is not a table or mixed style", style_name),
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        // Get or create table properties node
        pugi::xml_node tblpr = m_currentNode.child("w:tblPr");
        if (!tblpr) {
            tblpr = m_currentNode.prepend_child("w:tblPr");
        }
        
        if (!tblpr) {
            return Result<void>(errors::xml_manipulation_failed(
                "Failed to create table properties node",
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        // Apply style reference
        pugi::xml_node style_ref = tblpr.child("w:tblStyle");
        if (!style_ref) {
            style_ref = tblpr.append_child("w:tblStyle");
        }
        style_ref.attribute("w:val") ? 
            style_ref.attribute("w:val").set_value(style_name.c_str()) :
            style_ref.append_attribute("w:val").set_value(style_name.c_str());
        
        return Result<void>{};
    }

    Result<std::string> Table::get_style_safe() const
    {
        pugi::xml_node tblpr = m_currentNode.child("w:tblPr");
        if (!tblpr) {
            return Result<std::string>{std::string{}}; // No style applied
        }
        
        pugi::xml_node style_ref = tblpr.child("w:tblStyle");
        if (!style_ref) {
            return Result<std::string>{std::string{}}; // No style applied
        }
        
        pugi::xml_attribute val_attr = style_ref.attribute("w:val");
        if (!val_attr) {
            return Result<std::string>{std::string{}}; // No style value
        }
        
        return Result<std::string>{std::string(val_attr.value())};
    }

    Result<void> Table::remove_style_safe()
    {
        pugi::xml_node tblpr = m_currentNode.child("w:tblPr");
        if (!tblpr) {
            return Result<void>{}; // No properties to remove
        }
        
        pugi::xml_node style_ref = tblpr.child("w:tblStyle");
        if (style_ref) {
            tblpr.remove_child(style_ref);
        }
        
        return Result<void>{};
    }

} // namespace duckx