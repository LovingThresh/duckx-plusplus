// main.cpp - Create, save, then open and edit
#include <iostream>
#include "duckx.hpp"

int main() {
    try {
        // Step 1: Create empty document and save it
        std::cout << "Step 1: Creating empty document..." << std::endl;
        duckx::Document doc;
        if (doc.create("test.docx")) {
            std::cout << "Empty document created successfully" << std::endl;
            doc.save();
            std::cout << "Empty document saved" << std::endl;
        } else {
            std::cout << "Failed to create document" << std::endl;
            return 1;
        }

        // Step 2: Open the document we just created and add content
        std::cout << "\nStep 2: Opening the document to add content..." << std::endl;
        duckx::Document doc2("test.docx");
        doc2.open();

        if (doc2.is_open()) {
            std::cout << "Document opened successfully" << std::endl;

            // Use your exact working pattern
            duckx::Paragraph p = doc2.paragraphs().insert_paragraph_after("You can insert text in ");
            p.add_run("italic, ", duckx::italic);
            p.add_run("bold, ", duckx::bold);
            p.add_run("underline, ", duckx::underline);
            p.add_run("superscript", duckx::superscript);
            p.add_run(" or ");
            p.add_run("subscript, ", duckx::subscript);
            p.add_run("small caps, ", duckx::smallcaps);
            p.add_run("and shadows, ", duckx::shadow);
            p.add_run("and of course ");
            p.add_run("combine them.", duckx::bold | duckx::italic | duckx::underline | duckx::smallcaps);

            doc2.save();
            std::cout << "Content added and document saved successfully!" << std::endl;
        } else {
            std::cout << "Failed to open the created document" << std::endl;
            return 1;
        }

        // Step 3: Test creating another document with different approach
        std::cout << "\nStep 3: Testing alternative approach..." << std::endl;
        duckx::Document doc3;
        if (doc3.create("test2.docx")) {
            // Try to add content immediately after creation
            std::cout << "Trying to add content immediately after creation..." << std::endl;

            // Check if we can get paragraphs
            auto& para = doc3.paragraphs();
            if (para.has_next()) {
                std::cout << "Found existing paragraph, trying to add content..." << std::endl;
                // Try direct add_run on the first paragraph
                para.add_run("Direct text addition test", duckx::bold);
                doc3.save();
                std::cout << "Direct method test saved" << std::endl;
            } else {
                std::cout << "No paragraphs found in newly created document" << std::endl;
            }
        }

        std::cout << "\nAll tests completed!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}