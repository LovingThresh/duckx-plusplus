/*
 * @file: sample9.cpp
 * @brief: A sample demonstrating how to apply highlight colors to text.
 *
 * @author: liuye
 * @date: 2025.06.11
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include "duckx.hpp"
#include "test_utils.hpp"

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
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("sample9_highlighting.docx"));
        auto& body = doc.body();

        body.add_paragraph("Sample 9: Applying Highlight Colors")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph(); // Add a blank line for spacing

        // ==========================================================
        //  Test 1: Basic Highlighting
        // ==========================================================
        body.add_paragraph("This section demonstrates basic text highlighting:")
                .add_run("", duckx::bold);

        auto p1 = body.add_paragraph("This sentence contains ");
        p1.add_run("yellow highlighted text")
                .set_highlight(duckx::HighlightColor::YELLOW);
        p1.add_run(" and some ");
        p1.add_run("green highlighted text")
                .set_highlight(duckx::HighlightColor::GREEN);
        p1.add_run(".");

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 2: Highlighting Combined with Other Styles
        // ==========================================================
        body.add_paragraph("Highlighting can be combined with other formatting:")
                .add_run("", duckx::bold);

        auto p2 = body.add_paragraph();
        p2.add_run("This text is bold and highlighted in cyan.", duckx::bold)
                .set_highlight(duckx::HighlightColor::CYAN);

        body.add_paragraph(); // Another paragraph for clarity

        auto p3 = body.add_paragraph();
        p3.add_run("This text is italic, red, and highlighted in magenta.", duckx::italic)
                .set_color("FF0000")
                .set_highlight(duckx::HighlightColor::MAGENTA);

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 3: Showcase of Various Colors
        // ==========================================================
        body.add_paragraph("A showcase of various available highlight colors:")
                .add_run("", duckx::bold);

        auto p4 = body.add_paragraph("Available colors include: ");
        p4.add_run("Red").set_highlight(duckx::HighlightColor::RED);
        p4.add_run(", ");
        p4.add_run("Blue").set_highlight(duckx::HighlightColor::BLUE);
        p4.add_run(", ");
        p4.add_run("Dark Yellow").set_highlight(duckx::HighlightColor::DARK_YELLOW);
        p4.add_run(", ");
        p4.add_run("Light Gray").set_highlight(duckx::HighlightColor::LIGHT_GRAY);
        p4.add_run(", and ");
        p4.add_run("None").set_highlight(duckx::HighlightColor::NONE); // This should have no highlight
        p4.add_run(".");

        // 4. Save the document
        doc.save();

        std::cout << "Successfully created 'sample9_highlighting.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
