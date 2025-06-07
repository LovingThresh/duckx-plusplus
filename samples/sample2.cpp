#include <iostream>
#include "duckx.hpp"

int main()
{
    try
    {
        std::cout << "Step 1: Creating new document..." << std::endl;

        auto doc = duckx::Document::create("test.docx");
        std::cout << "Document object created successfully." << std::endl;

        doc.save();
        std::cout << "Empty document 'test.docx' saved." << std::endl;

        std::cout << "\nStep 2: Re-opening the document to add content..." << std::endl;

        auto doc2 = duckx::Document::open("test.docx");
        std::cout << "Document 'test.docx' opened successfully." << std::endl;

        auto p = doc2.body().add_paragraph("You can insert text in ");

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

        std::cout << "\nStep 3: Testing immediate modification after creation..." << std::endl;

        auto doc3 = duckx::Document::create("test2.docx");
        std::cout << "Document 'test2.docx' created." << std::endl;

        doc3.body().add_paragraph("This content was added immediately after creation.", duckx::bold);
        std::cout << "Content added directly to new document." << std::endl;

        doc3.save();
        std::cout << "Document 'test2.docx' saved." << std::endl;


        std::cout << "\nAll tests completed successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
