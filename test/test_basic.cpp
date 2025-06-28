#include <sstream>
#include "Document.hpp"
#include "gtest/gtest.h"

TEST(DocxContentTest, ChecksContentsOfMyTestFile)
{
    auto doc = duckx::Document::open("my_test.docx");

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
