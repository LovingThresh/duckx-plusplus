/*
 * @file: sample10.cpp
 * @brief: A sample demonstrating how to create and populate tables.
 *
 * @author: liuye
 * @date: 2025.06.11
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include <string> // For std::to_string
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
        duckx::Document doc = duckx::Document::create("sample10_tables.docx");
        auto& body = doc.body();

        body.add_paragraph("Sample 10: Working with Tables")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 1: Create and populate a simple table
        // ==========================================================
        body.add_paragraph("This is a simple 3x4 table:")
                .add_run("", duckx::bold);

        // Add a table with 3 rows and 4 columns
        auto table1 = body.add_table(3, 4);

        // Iterate through rows and cells to populate the table
        int row_num = 1;
        for (auto& row: table1.rows())
        {
            int col_num = 1;
            for (auto& cell: row.cells())
            {
                // Each new cell contains one empty paragraph. We get it to add content.
                auto& p = *cell.paragraphs().begin();
                p.add_run("Row " + std::to_string(row_num) + ", Col " + std::to_string(col_num));
                col_num++;
            }
            row_num++;
        }

        body.add_paragraph(); // Spacer

        // ==========================================================
        //  Test 2: Create a table with styled content
        // ==========================================================
        body.add_paragraph("This table demonstrates styled content inside cells:")
                .add_run("", duckx::bold);

        auto table2 = body.add_table(2, 2);

        // We'll populate this table with different styles
        auto& row1 = *table2.rows().begin();
        auto cell_it = row1.cells().begin();

        // Cell (0, 0)
        cell_it->paragraphs().begin()->add_run("Bold Text", duckx::bold);
        ++cell_it;

        // Cell (0, 1)
        cell_it->paragraphs().begin()
                ->add_run("Red Italic Text", duckx::italic)
                .set_color("FF0000");

        // Move to the next row
        auto& row2 = *(++table2.rows().begin());
        cell_it = row2.cells().begin();

        // Cell (1, 0)
        cell_it->paragraphs().begin()
                ->add_run("Highlighted Text")
                .set_highlight(duckx::HighlightColor::YELLOW);
        ++cell_it;

        // Cell (1, 1)
        auto& last_cell_p = *cell_it->paragraphs().begin();
        last_cell_p.add_run("Multi-style: ");
        last_cell_p.add_run("Bold, ", duckx::bold);
        last_cell_p.add_run("Underlined, ", duckx::underline);
        last_cell_p.add_run("and Green.").set_color("008000");

        // 4. Save the document
        doc.save();

        std::cout << "Successfully created 'sample10_tables.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
