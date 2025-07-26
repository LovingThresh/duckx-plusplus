/*
 * @file: sample14.cpp
 * @brief:
 * 
 * @author: liuye
 * @date: 2025.06.20
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include <string>
#include <vector>
#include <iomanip>
#include "duckx.hpp"
#include "test_utils.hpp"

// ==========================================================
//  Helper functions to make output readable
// ==========================================================

std::string format_flags_to_string(const duckx::formatting_flag flags)
{
    if (flags == duckx::none)
    {
        return "None";
    }
    std::vector<std::string> flag_names;
    if (flags & duckx::bold) flag_names.emplace_back("Bold");
    if (flags & duckx::italic) flag_names.emplace_back("Italic");
    if (flags & duckx::underline) flag_names.emplace_back("Underline");
    if (flags & duckx::strikethrough) flag_names.emplace_back("Strikethrough");
    if (flags & duckx::superscript) flag_names.emplace_back("Superscript");
    if (flags & duckx::subscript) flag_names.emplace_back("Subscript");
    if (flags & duckx::smallcaps) flag_names.emplace_back("SmallCaps");
    if (flags & duckx::shadow) flag_names.emplace_back("Shadow");

    std::string result;
    for (size_t i = 0; i < flag_names.size(); ++i)
    {
        result += flag_names[i] + (i < flag_names.size() - 1 ? ", " : "");
    }
    return result;
}

std::string highlight_to_string(const duckx::HighlightColor color)
{
    static const std::map<duckx::HighlightColor, std::string> color_map = {
        {duckx::HighlightColor::BLACK, "Black"}, {duckx::HighlightColor::BLUE, "Blue"},
        {duckx::HighlightColor::CYAN, "Cyan"}, {duckx::HighlightColor::GREEN, "Green"},
        {duckx::HighlightColor::MAGENTA, "Magenta"}, {duckx::HighlightColor::RED, "Red"},
        {duckx::HighlightColor::YELLOW, "Yellow"}, {duckx::HighlightColor::WHITE, "White"},
        {duckx::HighlightColor::DARK_BLUE, "DarkBlue"}, {duckx::HighlightColor::DARK_CYAN, "DarkCyan"},
        {duckx::HighlightColor::DARK_GREEN, "DarkGreen"}, {duckx::HighlightColor::DARK_MAGENTA, "DarkMagenta"},
        {duckx::HighlightColor::DARK_RED, "DarkRed"}, {duckx::HighlightColor::DARK_YELLOW, "DarkYellow"},
        {duckx::HighlightColor::LIGHT_GRAY, "LightGray"}
    };
    const auto it = color_map.find(color);
    return (it != color_map.end()) ? it->second : "Unknown";
}

// ==========================================================
//  Main analysis function for a paragraph
// ==========================================================
void analyze_paragraph(duckx::Paragraph& p)
{
    // ---- Paragraph-level formatting ----
    std::cout << "  [Paragraph Properties]" << std::endl;

    const duckx::Alignment align = p.get_alignment();
    std::string align_str = "Left";
    if (align == duckx::Alignment::CENTER) align_str = "Center";
    else if (align == duckx::Alignment::RIGHT) align_str = "Right";
    else if (align == duckx::Alignment::BOTH) align_str = "Justified";
    std::cout << "    - Alignment: " << align_str << std::endl;

    double before_pts, after_pts;
    if (p.get_spacing(before_pts, after_pts))
    {
        std::cout << "    - Spacing (pts): Before=" << before_pts << ", After=" << after_pts << std::endl;
    }

    double line_spacing;
    if (p.get_line_spacing(line_spacing))
    {
        std::cout << "    - Line Spacing: " << std::fixed << std::setprecision(2) << line_spacing << " lines" <<
                std::endl;
    }

    double left_pts, right_pts, first_line_pts;
    if (p.get_indentation(left_pts, right_pts, first_line_pts))
    {
        std::cout << "    - Indentation (pts): Left=" << left_pts << ", Right=" << right_pts << ", FirstLine=" <<
                first_line_pts << std::endl;
    }

    duckx::ListType list_type;
    int list_level, list_id;
    if (p.get_list_style(list_type, list_level, list_id))
    {
        std::cout << "    - List Style: Yes (Level=" << list_level << ", ID=" << list_id << ")" << std::endl;
    }

    // ---- Run-level formatting ----
    std::cout << "  [Run Details]" << std::endl;
    for (auto& r: p.runs())
    {
        std::string text = r.get_text();
        if (text.empty()) continue;

        std::cout << "    - Text: \"" << text << "\"" << std::endl;
        std::cout << "      - Formatting Flags: " << format_flags_to_string(r.get_formatting()) << std::endl;

        std::string font_name;
        if (r.get_font(font_name))
        {
            std::cout << "      - Font Name: " << font_name << std::endl;
        }

        double font_size;
        if (r.get_font_size(font_size))
        {
            std::cout << "      - Font Size: " << font_size << "pt" << std::endl;
        }

        std::string color_hex;
        if (r.get_color(color_hex))
        {
            std::cout << "      - Font Color (Hex): " << color_hex << std::endl;
        }

        duckx::HighlightColor highlight;
        if (r.get_highlight(highlight))
        {
            std::cout << "      - Highlight Color: " << highlight_to_string(highlight) << std::endl;
        }
    }
}

int main()
{
    try
    {
        std::locale::global(std::locale(""));
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Warning: Could not set system default locale. " << e.what() << std::endl;
    }

    try
    {
        // 1. Create a document with various paragraph formatting for analysis
        std::cout << "Creating test document for analysis..." << std::endl;
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("sample14_analysis_test.docx"));
        auto& body = doc.body();
        
        // Add sample content with various formatting
        body.add_paragraph("Sample 14: Document Analysis Demo").add_run("", duckx::bold);
        body.add_paragraph(); // Empty paragraph
        auto p_spaced = body.add_paragraph("This paragraph has extra spacing before and after it.");
        p_spaced.set_spacing(12, 18);
        body.add_paragraph("Notice the gap between this paragraph and the one above.");
        body.add_paragraph(); // Empty paragraph
        auto p_double = body.add_paragraph("This paragraph demonstrates double line spacing with longer text to show the effect clearly.");
        p_double.set_line_spacing(2.0);
        body.add_paragraph(); // Empty paragraph
        auto p_indented = body.add_paragraph("This quote is indented from both margins for emphasis.");
        p_indented.set_indentation(36.0, 36.0);
        
        doc.save();
        std::cout << "Test document created and saved." << std::endl;
        
        // 2. Now reopen the document for analysis
        std::cout << "Reopening document for analysis..." << std::endl;
        doc = duckx::Document::open(duckx::test_utils::get_temp_path("sample14_analysis_test.docx"));

        std::cout << "Document opened. Starting analysis of paragraphs.\n" << std::endl;

        // ==========================================================
        //  Test Run: Iterate through all paragraphs and analyze them
        // ==========================================================
        int p_index = 1;
        for (auto& p: body.paragraphs())
        {
            std::cout << "-------------------- Analyzing Paragraph " << p_index++ << " --------------------" <<
                    std::endl;
            analyze_paragraph(p);
        }

        std::cout << "\n--------------------------------------------------------------" << std::endl;
        std::cout << "Successfully analyzed document with paragraph formatting." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
