/*!
 * @file sample30_export_runs_to_txt.cpp
 * @brief Export DOCX run text to a TXT file, one run per line
 *
 * Usage:
 *   sample30_export_runs_to_txt input.docx output.txt
 *
 * Each output line is formatted as:
 *   <Run Begin>run text<Run End>
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

    void export_runs_to_txt(const std::string& input_docx, const std::string& output_txt)
    {
        duckx::Document doc = duckx::Document::open(input_docx);

        std::ofstream output(output_txt.c_str(), std::ios::out | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("Failed to open output file: " + output_txt);
        }

        for (auto& paragraph : doc.body().paragraphs())
        {
            for (const auto& run : paragraph.runs())
            {
                output << "<Run Begin>";
                write_escaped_text(output, run.get_text());
                output << "<Run End>\n";
            }
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
        export_runs_to_txt(argv[1], argv[2]);
        std::cout << "Exported runs to: " << argv[2] << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
