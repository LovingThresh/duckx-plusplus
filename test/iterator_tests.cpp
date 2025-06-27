#include <sstream>
#include <iostream>
#include "Document.hpp"
#include "duckxiterator.hpp"
#include "gtest/gtest.h"

TEST(DocxIteratorTest, RangeForLoopReadsContentCorrectly)
{
    // 2. 使用静态工厂方法打开文档
    auto doc = duckx::Document::open("my_test.docx");

    std::ostringstream ss;

    for (duckx::Paragraph& p : doc.body().paragraphs())
    {
        for (const duckx::Run& r : p.runs())
        {
            ss << r.get_text() << std::endl;
        }
    }

    const std::string expected_content = "This is a test\nokay?\n";
    EXPECT_EQ(expected_content, ss.str());
}

TEST(IteratorEqualityTest, ComparesIteratorsCorrectly)
{
    auto doc = duckx::Document::open("my_test.docx");

    auto paragraphs_range = doc.body().paragraphs();

    EXPECT_EQ(paragraphs_range.begin(), paragraphs_range.begin());

    EXPECT_NE(paragraphs_range.begin(), paragraphs_range.end());

    auto it1 = paragraphs_range.begin();
    auto it2 = paragraphs_range.begin();

    ++it1;
    EXPECT_NE(it1, it2);

    ++it2;
    EXPECT_EQ(it1, it2);
}
