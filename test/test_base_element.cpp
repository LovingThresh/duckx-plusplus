/*!
 * @file test_base_element.cpp
 * @brief Unit tests for BaseElement classes and document structure
 * 
 * Tests core element hierarchy including paragraphs, runs, tables,
 * rows, and cells with both modern Result<T> and legacy APIs.
 * Validates element navigation, manipulation, and formatting capabilities.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <BaseElement.hpp>
#include <chrono>
#include <Document.hpp>
#include <string>

namespace dx = duckx;

class BaseElementTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* xml_content = R"(
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
                <w:body>
                    <w:p>
                        <w:r>
                            <w:rPr>
                                <w:b/>
                                <w:sz w:val="24"/>
                                <w:color w:val="FF0000"/>
                                <w:highlight w:val="yellow"/>
                                <w:rFonts w:ascii="Arial"/>
                            </w:rPr>
                            <w:t>Hello World</w:t>
                        </w:r>
                        <w:r>
                            <w:t>Second run</w:t>
                        </w:r>
                    </w:p>
                    <w:p>
                        <w:pPr>
                            <w:jc w:val="center"/>
                            <w:spacing w:before="240" w:after="120"/>
                            <w:ind w:left="720" w:right="360" w:firstLine="360"/>
                        </w:pPr>
                        <w:r>
                            <w:t>Second paragraph</w:t>
                        </w:r>
                    </w:p>
                    <w:tbl>
                        <w:tr>
                            <w:tc>
                                <w:p>
                                    <w:r>
                                        <w:t>Cell 1</w:t>
                                    </w:r>
                                </w:p>
                            </w:tc>
                            <w:tc>
                                <w:p>
                                    <w:r>
                                        <w:t>Cell 2</w:t>
                                    </w:r>
                                </w:p>
                            </w:tc>
                        </w:tr>
                        <w:tr>
                            <w:tc>
                                <w:p>
                                    <w:r>
                                        <w:t>Cell 3</w:t>
                                    </w:r>
                                </w:p>
                            </w:tc>
                            <w:tc>
                                <w:p>
                                    <w:r>
                                        <w:t>Cell 4</w:t>
                                    </w:r>
                                </w:p>
                            </w:tc>
                        </w:tr>
                    </w:tbl>
                </w:body>
            </w:document>
        )";

        const auto result = doc.load_string(xml_content);
        ASSERT_TRUE(result) << "Failed to load test XML: " << result.description();

        body = doc.child("w:document").child("w:body");
        ASSERT_TRUE(body) << "Failed to find document body";
    }

    pugi::xml_document doc;
    pugi::xml_node body;
};

class RunTest : public BaseElementTest {};

TEST_F(RunTest, BasicConstruction)
{
    const auto para_node = body.child("w:p");
    const auto run_node = para_node.child("w:r");

    const dx::Run duckx_run(para_node, run_node);

    EXPECT_TRUE(duckx_run.get_node());
    EXPECT_EQ(duckx_run.get_text(), "Hello World");
}

TEST_F(RunTest, TextOperations)
{
    const auto para_node = body.child("w:p");
    const auto run_node = para_node.child("w:r");

    const dx::Run duckx_run(para_node, run_node);

    EXPECT_EQ(duckx_run.get_text(), "Hello World");

    EXPECT_TRUE(duckx_run.set_text("New Text"));
    EXPECT_EQ(duckx_run.get_text(), "New Text");

    EXPECT_TRUE(duckx_run.set_text("Another Text"));
    EXPECT_EQ(duckx_run.get_text(), "Another Text");
}

TEST_F(RunTest, FormattingOperations)
{
    const auto para_node = body.child("w:p");
    const auto run_node = para_node.child("w:r");

    const dx::Run duckx_run(para_node, run_node);

    EXPECT_TRUE(duckx_run.is_bold());
    EXPECT_FALSE(duckx_run.is_italic());
    EXPECT_FALSE(duckx_run.is_underline());

    std::string font_name;
    EXPECT_TRUE(duckx_run.get_font(font_name));
    EXPECT_EQ(font_name, "Arial");

    double font_size;
    EXPECT_TRUE(duckx_run.get_font_size(font_size));
    EXPECT_EQ(font_size, 12.0); // 24 half-points = 12 points

    std::string color;
    EXPECT_TRUE(duckx_run.get_color(color));
    EXPECT_EQ(color, "FF0000");

    dx::HighlightColor highlight;
    EXPECT_TRUE(duckx_run.get_highlight(highlight));
    EXPECT_EQ(highlight, dx::HighlightColor::YELLOW);
}

TEST_F(RunTest, SetFormatting)
{
    const auto para_node = body.child("w:p");
    const auto run_node = para_node.child("w:r").next_sibling("w:r"); // 第二个run

    dx::Run duckx_run(para_node, run_node);

    // 设置字体
    duckx_run.set_font("Times New Roman");
    std::string font_name;
    EXPECT_TRUE(duckx_run.get_font(font_name));
    EXPECT_EQ(font_name, "Times New Roman");

    // 设置字体大小
    duckx_run.set_font_size(14.0);
    double font_size;
    EXPECT_TRUE(duckx_run.get_font_size(font_size));
    EXPECT_EQ(font_size, 14.0);

    // 设置颜色
    duckx_run.set_color("0000FF");
    std::string color;
    EXPECT_TRUE(duckx_run.get_color(color));
    EXPECT_EQ(color, "0000FF");

    // 设置高亮
    duckx_run.set_highlight(dx::HighlightColor::GREEN);
    dx::HighlightColor highlight;
    EXPECT_TRUE(duckx_run.get_highlight(highlight));
    EXPECT_EQ(highlight, dx::HighlightColor::GREEN);
}

TEST_F(RunTest, Navigation)
{
    const auto para_node = body.child("w:p");
    const auto run_node = para_node.child("w:r");

    dx::Run duckx_run(para_node, run_node);

    // 测试是否有下一个
    EXPECT_TRUE(duckx_run.has_next());

    // 移动到下一个
    const dx::Run& next_run = duckx_run.advance();
    EXPECT_EQ(next_run.get_text(), "Second run");

    // 应该没有更多的run了
    EXPECT_FALSE(next_run.has_next());
}

// ==================== Paragraph类测试 ====================
class ParagraphTest : public BaseElementTest {};

TEST_F(ParagraphTest, BasicConstruction)
{
    const auto para_node = body.child("w:p");

    const dx::Paragraph para(body, para_node);

    EXPECT_TRUE(para.get_node());
    EXPECT_TRUE(para.has_next()); // 应该有第二个段落
}

TEST_F(ParagraphTest, RunOperations)
{
    const auto para_node = body.child("w:p");

    dx::Paragraph para(body, para_node);

    // 测试获取runs
    auto runs_range = para.runs();
    auto it = runs_range.begin();

    EXPECT_NE(it, runs_range.end());
    EXPECT_EQ(it->get_text(), "Hello World");

    ++it;
    EXPECT_NE(it, runs_range.end());
    EXPECT_EQ(it->get_text(), "Second run");

    ++it;
    EXPECT_EQ(it, runs_range.end());
}

TEST_F(ParagraphTest, AddRun)
{
    const auto para_node = body.child("w:p");

    dx::Paragraph para(body, para_node);

    // 添加新的run
    const dx::Run& new_run = para.add_run("New run text", dx::bold | dx::italic);
    EXPECT_EQ(new_run.get_text(), "New run text");
    EXPECT_TRUE(new_run.is_bold());
    EXPECT_TRUE(new_run.is_italic());

    // 添加run（const char*版本）
    const dx::Run& another_run = para.add_run("Another run");
    EXPECT_EQ(another_run.get_text(), "Another run");
}

TEST_F(ParagraphTest, ParagraphFormatting)
{
    const auto para_node = body.child("w:p").next_sibling("w:p"); // 第二个段落

    const dx::Paragraph para(body, para_node);

    // 测试对齐方式
    EXPECT_EQ(para.get_alignment(), dx::Alignment::CENTER);

    // 测试间距
    double before, after;
    EXPECT_TRUE(para.get_spacing(before, after));
    EXPECT_EQ(before, 12.0); // 240 twips = 12 points
    EXPECT_EQ(after, 6.0); // 120 twips = 6 points

    // 测试缩进
    double left, right, first_line;
    EXPECT_TRUE(para.get_indentation(left, right, first_line));
    EXPECT_EQ(left, 36.0); // 720 twips = 36 points
    EXPECT_EQ(right, 18.0); // 360 twips = 18 points
    EXPECT_EQ(first_line, 18.0); // 360 twips = 18 points
}

TEST_F(ParagraphTest, SetFormatting)
{
    const auto para_node = body.child("w:p");

    dx::Paragraph para(body, para_node);

    // 设置对齐方式
    para.set_alignment(dx::Alignment::RIGHT);
    EXPECT_EQ(para.get_alignment(), dx::Alignment::RIGHT);

    // 设置间距
    para.set_spacing(24.0, 12.0);
    double before, after;
    EXPECT_TRUE(para.get_spacing(before, after));
    EXPECT_EQ(before, 24.0);
    EXPECT_EQ(after, 12.0);

    // 设置行间距
    para.set_line_spacing(1.5);
    double line_spacing;
    EXPECT_TRUE(para.get_line_spacing(line_spacing));
    EXPECT_EQ(line_spacing, 1.5);

    // 设置缩进
    para.set_indentation(72.0, 36.0);
    para.set_first_line_indent(18.0);
    double left, right, first_line;
    EXPECT_TRUE(para.get_indentation(left, right, first_line));
    EXPECT_EQ(left, 72.0);
    EXPECT_EQ(right, 36.0);
    EXPECT_EQ(first_line, 18.0);
}

TEST_F(ParagraphTest, Navigation)
{
    const auto para_node = body.child("w:p");

    dx::Paragraph para(body, para_node);

    // 测试是否有下一个
    EXPECT_TRUE(para.has_next());

    // 移动到下一个段落
    const dx::Paragraph& next_para = para.advance();
    EXPECT_TRUE(next_para.get_node());

    // 应该还有表格，所以has_next可能返回false（取决于实现）
}

// ==================== Table相关测试 ====================
class TableTest : public BaseElementTest {};

TEST_F(TableTest, TableStructure)
{
    const auto table_node = body.child("w:tbl");

    const dx::Table table(body, table_node);

    EXPECT_TRUE(table.get_node());
    EXPECT_FALSE(table.has_next()); // 应该没有更多表格
}

TEST_F(TableTest, TableRows)
{
    auto table_node = body.child("w:tbl");

    dx::Table table(body, table_node);

    auto rows_range = table.rows();
    auto row_it = rows_range.begin();

    // 第一行
    EXPECT_NE(row_it, rows_range.end());

    auto cells_range = row_it->cells();
    auto cell_it = cells_range.begin();

    // 第一行第一列
    EXPECT_NE(cell_it, cells_range.end());
    auto paras_range = cell_it->paragraphs();
    auto para_it = paras_range.begin();
    EXPECT_NE(para_it, paras_range.end());

    auto runs_range = para_it->runs();
    auto run_it = runs_range.begin();
    EXPECT_EQ(run_it->get_text(), "Cell 1");

    // 第一行第二列
    ++cell_it;
    EXPECT_NE(cell_it, cells_range.end());
    paras_range = cell_it->paragraphs();
    para_it = paras_range.begin();
    runs_range = para_it->runs();
    run_it = runs_range.begin();
    EXPECT_EQ(run_it->get_text(), "Cell 2");

    // 第二行
    ++row_it;
    EXPECT_NE(row_it, rows_range.end());

    cells_range = row_it->cells();
    cell_it = cells_range.begin();

    // 第二行第一列
    paras_range = cell_it->paragraphs();
    para_it = paras_range.begin();
    runs_range = para_it->runs();
    run_it = runs_range.begin();
    EXPECT_EQ(run_it->get_text(), "Cell 3");

    // 第二行第二列
    ++cell_it;
    paras_range = cell_it->paragraphs();
    para_it = paras_range.begin();
    runs_range = para_it->runs();
    run_it = runs_range.begin();
    EXPECT_EQ(run_it->get_text(), "Cell 4");
}

// ==================== 边界条件和错误处理测试 ====================
class EdgeCaseTest : public BaseElementTest {};

TEST_F(EdgeCaseTest, EmptyNodes)
{
    const pugi::xml_node empty_node;
    const pugi::xml_node valid_node = body.child("w:p");

    // 测试空节点的处理
    const dx::Run duckx_run(valid_node, empty_node);
    EXPECT_FALSE(duckx_run.get_node());
    EXPECT_EQ(duckx_run.get_text(), "");
    EXPECT_FALSE(duckx_run.has_next());
}

TEST_F(EdgeCaseTest, InvalidOperations)
{
    const auto para_node = body.child("w:p");
    const auto run_node = para_node.child("w:r");

    const dx::Run duckx_run(para_node, run_node);

    // 测试无效的格式化获取
    std::string invalid_font;
    // 这取决于实现，可能返回false或默认值

    // 测试设置空文本
    EXPECT_TRUE(duckx_run.set_text(""));
    EXPECT_EQ(duckx_run.get_text(), "");
}

// ==================== 性能测试 ====================
class PerformanceTest : public BaseElementTest {};

TEST_F(PerformanceTest, LargeDocumentIteration)
{
    // 创建大量段落用于性能测试
    for (int i = 0; i < 100; ++i)
    {
        auto new_para = body.append_child("w:p");
        auto new_run = new_para.append_child("w:r");
        auto new_text = new_run.append_child("w:t");
        new_text.text().set(("Paragraph " + std::to_string(i)).c_str());
    }

    const auto para_node = body.child("w:p");
    dx::Paragraph para(body, para_node);

    int count = 0;
    const auto start = std::chrono::high_resolution_clock::now();

    while (para.has_next())
    {
        para.advance();
        count++;
        if (count > 1000) break; // 防止无限循环
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_GT(count, 90); // 应该迭代了大部分段落
    EXPECT_LT(duration.count(), 10000); // 应该在10ms内完成
}
