#include <sstream>      // 用于字符串流
#include "Document.hpp" // 引入你的 duckx 文档处理类
#include "gtest/gtest.h"  // 1. 引入 GTest 的核心头文件

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
