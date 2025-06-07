#include <sstream>
#include <iostream>
#include "Document.hpp"
#include "duckxiterator.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("checks contents of my_test.docx with C++ range-for loop")
{
    // 1. 使用新的静态工厂方法打开文档
    auto doc = duckx::Document::open("my_test.docx");

    std::ostringstream ss;

    // 2. 使用优雅的 for-each 循环！
    for (duckx::Paragraph& p : doc.body().paragraphs())
    {
        for (const duckx::Run& r : p.runs())
        {
            ss << r.get_text() << std::endl;
        }
    }

    std::puts(ss.str().c_str());
    CHECK_EQ("This is a test\n okay?\n", ss.str());
}

TEST_CASE("Check iterator equality")
{
    auto doc = duckx::Document::open("my_test.docx");

    // 获取段落的范围对象
    auto paragraphs_range = doc.body().paragraphs();

    // 1. begin() 迭代器应该等于它自己
    CHECK(paragraphs_range.begin() == paragraphs_range.begin());

    // 2. 对于非空文档，begin() 不应等于 end()
    CHECK(paragraphs_range.begin() != paragraphs_range.end());

    // 3. 我们可以手动迭代并比较
    auto it1 = paragraphs_range.begin();
    auto it2 = paragraphs_range.begin();

    // 移动一个迭代器
    ++it1;
    // 现在它们应该不相等
    CHECK(it1 != it2);

    // 再次移动第二个迭代器
    ++it2;
    // 现在它们应该再次相等
    CHECK(it1 == it2);
}