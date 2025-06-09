/*
 * @file: sample11.cpp
 * @brief: A sample demonstrating how to add and manipulate all types of headers and footers.
 *
 * @author: liuye
 * @date: 2025.06.12
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include <string>
#include "duckx.hpp"

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
        duckx::Document doc = duckx::Document::create("sample11_headers_footers.docx");
        auto& body = doc.body();

        body.add_paragraph("Sample 11: All Header & Footer Types")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph(
                "This document demonstrates all header/footer types: First, Default, Even, and Odd pages.");
        body.add_paragraph(
                "To see Even/Odd pages, you need to enable 'Different Odd & Even Pages' in Word's layout options.");

        // ==========================================================
        //  Test 1: Create a header for the FIRST page
        // ==========================================================
        auto& first_header = doc.get_header(duckx::HeaderFooterType::FIRST);
        first_header.add_paragraph("COMPANY MAIN TITLE")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(20);

        // ==========================================================
        //  Test 2: Create a header for ODD pages (e.g., page 3, 5, ...)
        // ==========================================================
        auto& odd_header = doc.get_header(duckx::HeaderFooterType::ODD);
        odd_header.add_paragraph("Chapter Title - Odd Pages")
                .set_alignment(duckx::Alignment::RIGHT);

        // ==========================================================
        //  Test 3: Create a header for EVEN pages (e.g., page 2, 4, ...)
        // ==========================================================
        auto& even_header = doc.get_header(duckx::HeaderFooterType::EVEN);
        even_header.add_paragraph("Book Title - Even Pages")
                .set_alignment(duckx::Alignment::LEFT);

        // ==========================================================
        //  Test 4: Create a default footer (will be used if odd/even are not set)
        // ==========================================================
        auto& default_footer = doc.get_footer(duckx::HeaderFooterType::DEFAULT);
        // Using a table for layout without complex iterators
        auto footer_table = default_footer.add_table(1, 2);

        // To access cells without iterators, we can re-query the elements.
        // This is less efficient but simpler to read in a sample.
        auto first_row = footer_table.rows().begin();
        auto first_cell_p = first_row->cells().begin()->paragraphs().begin();
        first_cell_p->add_run("Author: DuckX Team");

        auto second_cell_p = (++(first_row->cells().begin()))->paragraphs().begin();
        second_cell_p->add_run("Default Footer");
        second_cell_p->set_alignment(duckx::Alignment::RIGHT);

        // Add some content to the body to create multiple pages
        body.add_paragraph();
        for (int i = 0; i < 40; ++i)
        {
            body.add_paragraph(
                    "This is a filler paragraph to demonstrate how the header and footer appear on different pages. This is line "
                    + std::to_string(i + 1) + ".");
        }
        // In Word, after opening this document, you may need to go to
        // Layout -> Page Setup -> Layout (tab) and check "Different odd and even pages"
        // for the odd/even headers to display correctly. Our code sets up the files,
        // but this Word setting controls the final rendering.

        // 4. Save the document
        doc.save();

        std::cout << "Successfully created 'sample11_headers_footers.docx'." << std::endl;
        std::cout << "Please open the file to verify the different first, odd, and even page headers/footers." <<
                std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
