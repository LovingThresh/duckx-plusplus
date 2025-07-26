/*
 * @file: sample12.cpp
 * @brief: A sample to verify the shared rId management across different managers.
 *
 * This test is critical. It ensures that adding elements requiring relationships
 * (like images from MediaManager and headers from HeaderFooterManager)
 * in an interleaved manner does not cause rId conflicts.
 *
 * @author: liuye
 * @date: 2025.06.13
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include <stdexcept>
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
        duckx::Document doc = duckx::Document::create(duckx::test_utils::get_temp_path("sample12_rid_conflict_test.docx"));
        auto& body = doc.body();

        body.add_paragraph("Sample 12: rId Conflict Verification Test")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph("This document tests the core resource ID allocation mechanism.");
        body.add_paragraph(
            "If this document opens correctly in Microsoft Word, it means that the centralized 'rId' management in the Document class is working, and different managers (MediaManager, HeaderFooterManager) are correctly sharing and incrementing the ID counter without conflicts.");
        body.add_paragraph();

        // ==========================================================
        //  The Core Test: Interleaved resource allocation
        // ==========================================================

        std::cout << "Step 1: Adding a header (requires one rId)..." << std::endl;
        auto& header = doc.get_header();
        header.add_paragraph("This is the first resource: a header.");

        std::cout << "Step 2: Adding an image (requires another rId)..." << std::endl;
        // Assume you have an image file named 'logo.png' in the execution directory.
        // Copy one from other samples if needed.
        try
        {
            duckx::Image image("logo.png");
            auto p = body.add_paragraph();
            p.add_run("An image from MediaManager: ");
            p.add_run("");
            doc.media().add_image(p, image);
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Warning: Could not add image. Ensure 'logo.png' exists. " << e.what() << std::endl;
            body.add_paragraph("Image 'logo.png' not found, but continuing test...");
        }

        std::cout << "Step 3: Adding a footer (requires a third rId)..." << std::endl;
        auto& footer = doc.get_footer();
        footer.add_paragraph("This is the third resource: a footer.");

        std::cout << "Step 4: Adding a second image (requires a fourth rId)..." << std::endl;
        try
        {
            const duckx::Image image2("logo.png");
            auto p = body.add_paragraph("A second image to ensure counter continues to increment:");
            p.add_run("");
            doc.media().add_image(p, image2);
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Warning: Could not add second image. Ensure 'logo.png' exists. " << e.what() << std::endl;
        }

        // 4. Save the document
        doc.save();

        std::cout << "\nSuccessfully created 'sample12_rid_conflict_test.docx'." << std::endl;
        std::cout
                << "Verification PASSED if the document can be opened without errors."
                << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << std::endl;
        std::cout << "Verification FAILED." << std::endl;
        return 1;
    }

    return 0;
}
