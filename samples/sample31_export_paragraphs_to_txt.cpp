/*!
 * @file sample31_export_paragraphs_to_txt.cpp
 * @brief Export DOCX paragraph text to a TXT file, one paragraph per line
 *
 * Usage:
 *   sample31_export_paragraphs_to_txt input.docx output.txt
 *
 * Each output line is formatted as:
 *   <Paragraph Begin>paragraph text<Paragraph End>
 *
 * @date 2026.05
 */

#include <duckx.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    void write_escaped_text(std::ostream& os, const std::string& text)
    {
        for (const char ch : text)
        {
            switch (ch)
            {
                case '\r':
                    os << "\\r";
                    break;
                case '\n':
                    os << "\\n";
                    break;
                default:
                    os << ch;
                    break;
            }
        }
    }

    void export_paragraphs_to_txt(const std::string& input_docx, const std::string& output_txt)
    {
        duckx::Document doc = duckx::Document::open(input_docx);

        std::ofstream output(output_txt.c_str(), std::ios::out | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("Failed to open output file: " + output_txt);
        }

        for (auto& paragraph : doc.body().paragraphs())
        {
            output << "<Paragraph Begin>";
            for (const auto& run : paragraph.runs())
            {
                write_escaped_text(output, run.get_text());
            }
            output << "<Paragraph End>\n";
        }
    }
}

int main(const int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " input.docx output.txt" << std::endl;
        return 1;
    }

    try
    {
        export_paragraphs_to_txt(argv[1], argv[2]);
        std::cout << "Exported paragraphs to: " << argv[2] << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
