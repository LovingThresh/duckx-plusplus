/*
 * @file: sample15.cpp
 * @brief: Table formatting sample demonstrating modern Result<T> API
 *
 * @author: duckx-custom team
 * @date: 2025.07.24
 */

#include <duckx.hpp>
#include <iostream>
#include "test_utils.hpp"

using namespace duckx;

int main()
{
    try
    {
        // Create a new document using modern Result<T> API
        auto doc_result = Document::create_safe(test_utils::get_temp_path("sample15_table_formatting.docx"));
        if (!doc_result.ok())
        {
            std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
            return 1;
        }
        auto doc = std::move(doc_result.value());

        // Add a title paragraph
        auto& body = doc.body();
        auto title_para = body.add_paragraph("Table Formatting Sample", bold | italic);
        title_para.set_alignment(Alignment::CENTER);

        // Create a table with 3 rows and 4 columns
        auto table = body.add_table(3, 4);

        // Configure table using traditional API since safe methods don't exist
        table.set_width(400.0);
        table.set_alignment("center");
        table.set_border_style("single");
        table.set_border_width(1.0);
        table.set_border_color("000000");
        table.set_cell_margins(5.0, 5.0, 5.0, 5.0);

        // Fill table with data using modern API
        int row_index = 0;
        for (auto& row: table.rows())
        {
            // Configure row using Result<T> API
            if (row_index == 0)
            {
                // Header row configuration
                auto row_result = row.set_height_safe(25.0);
                if (!row_result.ok())
                {
                    std::cerr << "Failed to set header row height: " << row_result.error().to_string() << std::endl;
                    return 1;
                }

                row_result = row_result.value()->set_height_rule_safe("exact");
                if (!row_result.ok())
                {
                    std::cerr << "Failed to set header row height rule: " << row_result.error().to_string()
                              << std::endl;
                    return 1;
                }

                row_result = row_result.value()->set_header_row_safe(true);
                if (!row_result.ok())
                {
                    std::cerr << "Failed to set header row: " << row_result.error().to_string() << std::endl;
                    return 1;
                }
            }
            else
            {
                // Data row configuration
                auto row_result = row.set_height_safe(20.0);
                if (!row_result.ok())
                {
                    std::cerr << "Failed to set data row height: " << row_result.error().to_string() << std::endl;
                    return 1;
                }

                row_result = row_result.value()->set_height_rule_safe("atLeast");
                if (!row_result.ok())
                {
                    std::cerr << "Failed to set data row height rule: " << row_result.error().to_string() << std::endl;
                    return 1;
                }
            }

            int cell_index = 0;
            for (auto& cell: row.cells())
            {
                // Generate content
                std::string content;
                if (row_index == 0)
                {
                    const char* headers[] = {"Name", "Age", "City", "Score"};
                    content = headers[cell_index];
                }
                else
                {
                    switch (cell_index)
                    {
                        case 0:
                            content = "Person " + std::to_string(row_index);
                            break;
                        case 1:
                            content = std::to_string(20 + row_index * 5);
                            break;
                        case 2:
                            content = (row_index == 1) ? "New York" : "London";
                            break;
                        case 3:
                            content = std::to_string(85 + row_index * 3);
                            break;
                    }
                }

                // Configure cell using Result<T> API
                auto cell_result = cell.set_width_safe(100.0);
                if (!cell_result.ok())
                {
                    std::cerr << "Failed to set cell width: " << cell_result.error().to_string() << std::endl;
                    return 1;
                }

                cell_result = cell_result.value()->set_width_type_safe("dxa");
                if (!cell_result.ok())
                {
                    std::cerr << "Failed to set cell width type: " << cell_result.error().to_string() << std::endl;
                    return 1;
                }

                cell_result = cell_result.value()->set_vertical_alignment_safe(row_index == 0 ? "center" : "top");
                if (!cell_result.ok())
                {
                    std::cerr << "Failed to set cell vertical alignment: " << cell_result.error().to_string()
                              << std::endl;
                    return 1;
                }

                if (row_index == 0)
                {
                    // Header cell styling
                    cell_result = cell_result.value()->set_background_color_safe("E0E0E0");
                    if (!cell_result.ok())
                    {
                        std::cerr << "Failed to set cell background color: " << cell_result.error().to_string()
                                  << std::endl;
                        return 1;
                    }

                    cell_result = cell_result.value()->set_border_style_safe("single");
                    if (!cell_result.ok())
                    {
                        std::cerr << "Failed to set cell border style: " << cell_result.error().to_string()
                                  << std::endl;
                        return 1;
                    }

                    cell_result = cell_result.value()->set_border_width_safe(1.5);
                    if (!cell_result.ok())
                    {
                        std::cerr << "Failed to set cell border width: " << cell_result.error().to_string()
                                  << std::endl;
                        return 1;
                    }
                }
                else
                {
                    // Data cell styling
                    cell_result = cell_result.value()->set_border_style_safe("single");
                    if (!cell_result.ok())
                    {
                        std::cerr << "Failed to set cell border style: " << cell_result.error().to_string()
                                  << std::endl;
                        return 1;
                    }

                    cell_result = cell_result.value()->set_border_width_safe(0.5);
                    if (!cell_result.ok())
                    {
                        std::cerr << "Failed to set cell border width: " << cell_result.error().to_string()
                                  << std::endl;
                        return 1;
                    }
                }

                // Add paragraph to cell
                auto para = cell.add_paragraph();
                if (row_index == 0)
                {
                    para.add_run(content, bold);
                }
                else
                {
                    para.add_run(content);
                }

                cell_index++;
            }
            row_index++;
        }

        // Add another paragraph after the table
        auto para = body.add_paragraph("\nThis table demonstrates the new formatting capabilities:");
        para.set_alignment(Alignment::LEFT);

        // Create a list of features using modern API
        auto features_para = body.add_paragraph();
        features_para.add_run("✓ Modern Result<T> API with comprehensive error handling\n", bold);
        features_para.add_run("✓ Table width, alignment, and border configuration\n");
        features_para.add_run("✓ Row height and header row settings\n");
        features_para.add_run("✓ Cell width, alignment, and background colors\n");
        features_para.add_run("✓ Detailed border styling with width and color control\n");
        features_para.add_run("✓ Fluent interface with chainable method calls\n");
        features_para.add_run("✓ Comprehensive parameter validation and error reporting");

        // Save document using Result<T> API
        auto save_result = doc.save_safe();
        if (!save_result.ok())
        {
            std::cerr << "Failed to save document: " << save_result.error().to_string() << std::endl;
            return 1;
        }

        std::cout << "Document 'sample15_table_formatting.docx' created successfully!" << std::endl;
        std::cout << "This sample demonstrates the modern Result<T> API with comprehensive error handling."
                  << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
