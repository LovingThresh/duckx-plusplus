/*!
 * @file test_basic.cpp
 * @brief Basic functionality tests for DOCX document operations
 * 
 * Tests fundamental document reading, content extraction, and
 * text processing capabilities. Validates core library functionality
 * with basic use cases and content verification.
 * 
 * @date 2025.07
 */

#include <sstream>
#include "Document.hpp"
#include "gtest/gtest.h"
#include "test_utils.hpp"

TEST(DocxContentTest, ChecksContentsOfMyTestFile)
{
    auto doc = duckx::Document::open(duckx::test_utils::get_temp_path("my_test.docx"));

    std::ostringstream actual_content_stream;
    for (auto& p: doc.body().paragraphs())
    {
        for (const auto& r: p.runs())
        {
            actual_content_stream << r.get_text() << std::endl;
        }
    }

    const std::string expected_content = "This is a test\nokay?\n";
    EXPECT_EQ(expected_content, actual_content_stream.str());
}
