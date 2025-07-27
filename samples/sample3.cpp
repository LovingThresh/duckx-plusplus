/*!
 * @file sample3.cpp
 * @brief Image embedding and media handling demonstration
 * 
 * Demonstrates embedding images into DOCX documents with various
 * positioning options, sizing controls, and media management features.
 * Shows comprehensive image integration workflow.
 * 
 * @date 2025.07
 */
#include <iostream>

#include "duckx.hpp"
#include "test_utils.hpp"

int main()
{
    try
    {
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("sample3_test_with_image.docx"));
        const auto p = doc.body().add_paragraph("Company Logo:");
        const duckx::Image logo(duckx::test_utils::get_temp_path("logo.png"));
        doc.media().add_image(p, logo);
        doc.save();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
