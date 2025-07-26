/*
 * @file: sample13.cpp
 * @brief:
 * 
 * @author: liuy
 * @date: 2025.06.10
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
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("sqample13_hyperlinks.docx"));
        auto& body = doc.body();

        body.add_paragraph("Sample 12: Adding Hyperlinks")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 1: Create a simple paragraph with a hyperlink
        // ==========================================================
        body.add_paragraph("This test demonstrates a basic hyperlink.");

        auto p1 = body.add_paragraph("You can find more information on the ");

        // Here is the new API in action.
        // We pass the 'doc' object as the context to the method.
        p1.add_hyperlink(doc, "official DuckX GitHub page", "https://github.com/amiremohamadi/DuckX");

        p1.add_run(".");

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 2: A paragraph with multiple hyperlinks and text
        // ==========================================================
        body.add_paragraph("A single paragraph can contain multiple links.");

        auto p2 = body.add_paragraph("Popular search engines include ");
        p2.add_hyperlink(doc, "Google", "https://www.google.com");
        p2.add_run(", ");
        p2.add_hyperlink(doc, "Bing", "https://www.bing.com");
        p2.add_run(", and ");
        p2.add_hyperlink(doc, "DuckDuckGo", "https://www.duckduckgo.com");
        p2.add_run(".");

        // The returned Run from add_hyperlink can be styled, although it's
        // less common as it uses the default "Hyperlink" style.
        auto p3 = body.add_paragraph("This is a ", duckx::bold);
        auto link_run = p3.add_hyperlink(doc, "styled hyperlink", "http://example.com");

        // 4. Save the document
        doc.save();

        std::cout << "Successfully created 'sqample13_hyperlinks.docx'." << std::endl;
        std::cout << "Please open the file to verify the hyperlinks." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
