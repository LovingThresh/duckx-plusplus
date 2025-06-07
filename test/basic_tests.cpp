#include <sstream>
#include "Document.hpp"
#include "doctest.h"

TEST_CASE("checks contents of my_test.docx")
{
    // 1. 使用新的静态工厂方法打开文档
    auto doc = duckx::Document::open("my_test.docx");

    std::ostringstream ss;

    for (auto p : doc.body().paragraphs())
    {
        for (const auto& r : p.runs())
        {
            ss << r.get_text() << std::endl;
        }
    }

    CHECK_EQ("This is a test\nokay?\n", ss.str());
}
