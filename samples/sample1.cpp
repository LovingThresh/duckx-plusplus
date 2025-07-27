/*!
 * @file sample1.cpp
 * @brief Basic document reading and content extraction example
 * 
 * Demonstrates basic usage of DuckX-PLusPlus library for reading
 * DOCX documents and extracting text content using modern iterator
 * patterns and range-based loops.
 * 
 * @date 2025.07
 */

#include <duckx.hpp>
#include <iostream>
#include "test_utils.hpp"
using namespace std;

int main()
{
    try
    {
        auto doc = duckx::Document::open(duckx::test_utils::get_temp_path("my_test.docx"));

        std::cout << "--- Document Content (using for-each loop) ---" << std::endl;

        for (auto& p : doc.body().paragraphs())
        {
            for (auto& r : p.runs())
            {
                std::cout << r.get_text();
            }
            std::cout << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
