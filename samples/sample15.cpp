/*
 * @file: sample15.cpp
 * @brief: Table formatting methods demonstration
 * 
 * This sample shows how to use the Table, TableRow, and TableCell
 * formatting methods to create richly formatted tables.
 */

#include <iostream>
#include "duckx.hpp"

using namespace duckx;

int main()
{
    try {
        // Create a new document
        auto doc = Document::create("sample15_table_formatting.docx");

        // Add a title paragraph
        auto& body = doc.body();
        auto title_para = body.add_paragraph();
        title_para.add_run("Table Formatting Sample", bold | italic);
        title_para.set_alignment(Alignment::CENTER);

        // Create a table with 3 rows and 4 columns
        auto table = body.add_table(3, 4);

        // Format the entire table
        table.set_alignment(Alignment::CENTER)
             .set_width(400)  // 400 points width
             .set_border_style("single")
             .set_border_width(1.0)
             .set_border_color("0000FF")  // Blue borders
             .set_cell_margins(5, 5, 10, 10);  // top, bottom, left, right margins

        // Format table rows
        int row_index = 0;
        for (auto& row : table.rows()) {
            if (row_index == 0) {
                // Header row formatting
                row.set_height(30)
                   .set_height_rule("exact")
                   .set_header_row(true)
                   .set_cant_split(true);
            } else {
                // Data row formatting
                row.set_height(25)
                   .set_height_rule("atLeast");
            }

            // Format table cells
            int cell_index = 0;
            for (auto& cell : row.cells()) {
                if (row_index == 0) {
                    // Header cell formatting
                    cell.set_background_color("E6E6FA")  // Light lavender
                        .set_vertical_alignment("center")
                        .set_border_style("double")
                        .set_border_width(1.5)
                        .set_border_color("000080");  // Navy blue
                } else {
                    // Data cell formatting
                    cell.set_vertical_alignment("top")
                        .set_margins(3, 3, 8, 8);
                    
                    // Alternate row coloring
                    if (row_index % 2 == 0) {
                        cell.set_background_color("F0F8FF");  // Alice blue
                    }
                }

                // Add content to cells
                std::string content;
                if (row_index == 0) {
                    content = "Header " + std::to_string(cell_index + 1);
                } else {
                    content = "Row " + std::to_string(row_index) + 
                             ", Col " + std::to_string(cell_index + 1);
                }

                // Add paragraph to cell
                auto para = cell.add_paragraph();
                if (row_index == 0) {
                    para.add_run(content, bold);
                    para.set_alignment(Alignment::CENTER);
                } else {
                    para.add_run(content);
                    para.set_alignment(Alignment::LEFT);
                }

                cell_index++;
            }
            row_index++;
        }

        // Add another paragraph after the table
        auto para = body.add_paragraph();
        para.add_run("\nThis table demonstrates the new formatting capabilities:");
        para.set_alignment(Alignment::LEFT);

        // Create a list of features
        auto features_para = body.add_paragraph();
        features_para.add_run("• Table alignment and width control\n");
        features_para.add_run("• Border styling (style, width, color)\n");
        features_para.add_run("• Cell margin configuration\n");
        features_para.add_run("• Row height and header settings\n");
        features_para.add_run("• Cell background colors and vertical alignment\n");
        features_para.add_run("• Individual cell border customization");

        // Save the document
        doc.save();
        std::cout << "Table formatting sample created successfully: sample15_table_formatting.docx" << std::endl;

        // Demonstrate getter methods
        std::cout << "\nTable Properties:" << std::endl;
        std::cout << "Alignment: " << static_cast<int>(table.get_alignment()) << std::endl;
        
        double width;
        if (table.get_width(width)) {
            std::cout << "Width: " << width << " points" << std::endl;
        }

        std::string border_style;
        if (table.get_border_style(border_style)) {
            std::cout << "Border style: " << border_style << std::endl;
        }

        double border_width;
        if (table.get_border_width(border_width)) {
            std::cout << "Border width: " << border_width << " points" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
