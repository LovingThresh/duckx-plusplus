/*
 * @file: sample8.cpp
 * @brief: A sample demonstrating how to create bulleted and numbered lists.
 *
 * @author: liuye
 * @date: 2025.06.10
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include "duckx.hpp"
#include "test_utils.hpp"

int main()
{
    try {
        std::locale::global(std::locale(""));
    } catch (const std::runtime_error& e) {
        std::cerr << "Warning: Could not set system default locale. " << e.what() << std::endl;
    }

    try
    {
        // 1. Create a new .docx file
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("sample8_lists.docx"));
        auto& body = doc.body();

        body.add_paragraph("Sample 8: Creating Lists")
            .set_alignment(duckx::Alignment::CENTER)
            .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph();

        // ==========================================================
        //  Test 1: Simple Bulleted List
        // ==========================================================
        body.add_paragraph("This is a simple bulleted list:");

        body.add_paragraph("First item")
            .set_list_style(duckx::ListType::BULLET);

        body.add_paragraph("Second item")
            .set_list_style(duckx::ListType::BULLET);

        body.add_paragraph("Third item")
            .set_list_style(duckx::ListType::BULLET);

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 2: Simple Numbered List
        // ==========================================================
        body.add_paragraph("This is a simple numbered list:");

        body.add_paragraph(u8"第一项")
            .set_list_style(duckx::ListType::NUMBER);

        body.add_paragraph(u8"第二项")
            .set_list_style(duckx::ListType::NUMBER);

        body.add_paragraph(u8"第三项")
            .set_list_style(duckx::ListType::NUMBER);

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 3: Multi-level List
        // ==========================================================
        body.add_paragraph("And here is a multi-level list:");

        body.add_paragraph("Level 1, Item 1")
            .set_list_style(duckx::ListType::NUMBER, 0); // Level 0

        body.add_paragraph("Level 2, Item A")
            .set_list_style(duckx::ListType::NUMBER, 1); // Level 1 (indented)

        body.add_paragraph("Level 2, Item B")
            .set_list_style(duckx::ListType::NUMBER, 1); // Level 1

        body.add_paragraph("Level 3, Item i")
            .set_list_style(duckx::ListType::NUMBER, 2); // Level 2 (further indented)

        body.add_paragraph("Level 1, Item 2")
            .set_list_style(duckx::ListType::NUMBER, 0); // Back to Level 0

        // 4. Save the document
        doc.save();

        std::cout << "Successfully created 'sample8_lists.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}