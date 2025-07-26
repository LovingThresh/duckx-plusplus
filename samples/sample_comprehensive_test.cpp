/*
 * @file: sample_comprehensive_test.cpp
 * @brief: A comprehensive test demonstrating most of the library's features.
 *
 * @author: liuye
 * @date: 2025.06.14
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include <string>
#include "duckx.hpp"
#include "test_utils.hpp"

// A helper function to add a titled section to the document
void add_section_title(duckx::Body& body, const std::string& title)
{
    body.add_paragraph(); // Spacer
    body.add_paragraph(title)
            .set_alignment(duckx::Alignment::LEFT)
            .add_run("", duckx::bold | duckx::underline).set_font_size(14);
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
        // 1. Create a new .docx file
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("comprehensive_test.docx"));
        auto& body = doc.body();

        // ==========================================================
        //  Document Title
        // ==========================================================
        body.add_paragraph("DuckX Library: Comprehensive Feature Test")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(20);
        body.add_paragraph("Generated on: 2025-06-14")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::italic);

        // ==========================================================
        //  Section 1: Paragraphs and Text Formatting
        // ==========================================================
        add_section_title(body, "1. Paragraphs & Text Formatting");
        auto p_format = body.add_paragraph("This paragraph tests various formats: ");
        p_format.add_run("bold", duckx::bold);
        p_format.add_run(", ");
        p_format.add_run("italic", duckx::italic);
        p_format.add_run(", ");
        p_format.add_run("underlined", duckx::underline);
        p_format.add_run(", and ");
        p_format.add_run("strikethrough", duckx::strikethrough);
        p_format.add_run(".");

        auto p_style = body.add_paragraph("This run has ");
        p_style.add_run("red text").set_color("FF0000");
        p_style.add_run(" on a ");
        p_style.add_run("yellow background").set_highlight(duckx::HighlightColor::YELLOW);
        p_style.add_run(" with a different font.").set_font("Courier New").set_font_size(10);

        body.add_paragraph("This paragraph has right alignment and spacing.")
                .set_alignment(duckx::Alignment::RIGHT)
                .set_spacing(12, 12) // 12pt before, 12pt after
                .set_first_line_indent(36); // 0.5 inch indent

        // ==========================================================
        //  Section 2: Lists and Hyperlinks
        // ==========================================================
        add_section_title(body, "2. Lists & Hyperlinks");
        body.add_paragraph("A multi-level list:");
        body.add_paragraph("Level 1 Item").set_list_style(duckx::ListType::BULLET, 0);
        body.add_paragraph("Level 2 Item").set_list_style(duckx::ListType::NUMBER, 1);
        body.add_paragraph("Level 3 Item").set_list_style(duckx::ListType::BULLET, 2);

        auto p_link = body.add_paragraph("For more details, visit the ");
        p_link.add_hyperlink(doc, "DuckX Project Page", "https://github.com/amiremohamadi/DuckX");
        p_link.add_run(".");

        // ==========================================================
        //  Section 3: Tables
        // ==========================================================
        add_section_title(body, "3. Tables");
        auto table = body.add_table(2, 3);
        // Accessing cells directly without iterator variables
        table.rows().begin()->cells().begin()->paragraphs().begin()->add_run("Header 1", duckx::bold);
        (++(table.rows().begin()->cells().begin()))->paragraphs().begin()->add_run("Header 2", duckx::bold);
        (++(++(table.rows().begin()->cells().begin())))->paragraphs().begin()->add_run("Header 3", duckx::bold);

        // ==========================================================
        //  Section 4: Images and TextBoxes
        // ==========================================================
        add_section_title(body, "4. Images & Drawing Elements");
        body.add_paragraph("Below is an image and a textbox (if 'logo.png' is available).");
        try
        {
            duckx::Image img("logo.png"); // Assumes logo.png is in the execution directory
            duckx::TextBox tb(duckx::BorderStyle::SOLID);
            tb.add_new_paragraph("This is a test inside a textbox.");
            tb.add_paragraph("It supports paragraphs too!").add_run("", duckx::italic);

            auto p_drawings = body.add_paragraph();
            doc.media().add_image(p_drawings, img);
            p_drawings.add_run("   "); // Add space between drawings
            doc.media().add_textbox(p_drawings, tb);
        }
        catch (const std::exception& e)
        {
            body.add_paragraph("Could not load 'logo.png', skipping image/textbox test.").add_run("", duckx::italic);
            std::cerr << "Warning: " << e.what() << std::endl;
        }

        // ==========================================================
        //  Section 5: Headers and Footers
        // ==========================================================
        add_section_title(body, "5. Headers and Footers");

        // First Page Header
        doc.get_header(duckx::HeaderFooterType::FIRST)
                .add_paragraph("First Page Header: Special Title")
                .set_alignment(duckx::Alignment::CENTER);

        // Default Header (for even/odd pages if they are not set)
        doc.get_header(duckx::HeaderFooterType::DEFAULT)
                .add_paragraph("Default Document Header")
                .set_alignment(duckx::Alignment::RIGHT);

        // Even Page Footer
        doc.get_footer(duckx::HeaderFooterType::EVEN)
                .add_paragraph("Even Page Footer")
                .set_alignment(duckx::Alignment::LEFT);

        // Odd Page Footer
        doc.get_footer(duckx::HeaderFooterType::ODD)
                .add_paragraph("Odd Page Footer")
                .set_alignment(duckx::Alignment::RIGHT);

        body.add_paragraph(
                "The headers and footers have been set. This document is intentionally long to show them on multiple pages. Please enable 'Different Odd & Even Pages' in your Word processor to see the full effect.");

        // Add filler content to span multiple pages
        for (int i = 0; i < 50; ++i)
        {
            body.add_paragraph("Filler paragraph line " + std::to_string(i + 1) + " to extend the document length.");
        }

        // 6. Save the final document
        doc.save();

        std::cout << "Successfully created 'comprehensive_test.docx'." << std::endl;
        std::cout << "Please open the file to verify all features." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
