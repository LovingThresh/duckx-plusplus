#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Body.hpp"
#include "BaseElement.hpp"
#include "constants.hpp"
#include <memory>
#include <string>

class BodyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a minimal XML document for testing
        m_doc = std::make_unique<pugi::xml_document>();
        pugi::xml_node root = m_doc->append_child("w:document");
        root.append_attribute("xmlns:w") = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
        m_body_node = root.append_child("w:body");

        // Initialize Body with the created node
        m_body = std::make_unique<duckx::Body>(m_body_node);
    }

    void TearDown() override
    {
        while (m_body_node.first_child())
        {
            m_body_node.remove_child(m_body_node.first_child());
        }
        m_body.reset();
        m_doc.reset();
    }

    pugi::xml_node create_paragraph_node()
    {
        pugi::xml_node p_node = m_body_node.append_child("w:p");
        pugi::xml_node r_node = p_node.append_child("w:r");
        const pugi::xml_node t_node = r_node.append_child("w:t");
        t_node.text().set("Test text");
        return p_node;
    }

    pugi::xml_node create_table_node(const int rows = 2, const int cols = 2)
    {
        pugi::xml_node table_node = m_body_node.append_child("w:tbl");

        for (int i = 0; i < rows; ++i)
        {
            pugi::xml_node tr_node = table_node.append_child("w:tr");
            for (int j = 0; j < cols; ++j)
            {
                pugi::xml_node tc_node = tr_node.append_child("w:tc");
                pugi::xml_node p_node = tc_node.append_child("w:p");
                pugi::xml_node r_node = p_node.append_child("w:r");
                pugi::xml_node t_node = r_node.append_child("w:t");
                t_node.text().set("Cell content");
            }
        }
        return table_node;
    }

    std::unique_ptr<pugi::xml_document> m_doc;
    pugi::xml_node m_body_node;
    std::unique_ptr<duckx::Body> m_body;
};

TEST_F(BodyTest, DefaultConstructor)
{
    duckx::Body default_body;
}

TEST_F(BodyTest, ParameterizedConstructor_ValidNode)
{
    pugi::xml_document doc;
    const pugi::xml_node body_node = doc.append_child("w:body");

    duckx::Body body(body_node);
    // Constructor should complete without exception
    EXPECT_NO_THROW(duckx::Body test_body(body_node));
}

TEST_F(BodyTest, ParameterizedConstructor_InvalidNode)
{
    const pugi::xml_node invalid_node;

    // Constructor should handle invalid node gracefully
    EXPECT_NO_THROW(duckx::Body body(invalid_node));
}

// Paragraph functionality tests
TEST_F(BodyTest, Paragraphs_EmptyBody)
{
    auto paragraph_range = m_body->paragraphs();

    // Empty body should return empty range
    const auto it = paragraph_range.begin();
    const auto end = paragraph_range.end();
    EXPECT_EQ(it, end);
}

TEST_F(BodyTest, Paragraphs_WithExistingParagraphs)
{
    create_paragraph_node();
    create_paragraph_node();

    auto paragraph_range = m_body->paragraphs();
    const auto it = paragraph_range.begin();
    const auto end = paragraph_range.end();

    // Should be able to iterate through paragraphs
    EXPECT_NE(it, end);

    int count = 0;
    for (auto& p: paragraph_range)
    {
        ++count;
        EXPECT_TRUE(p.getNode());
    }
    EXPECT_EQ(count, 2);
}

TEST_F(BodyTest, AddParagraph_EmptyText)
{
    const auto paragraph = m_body->add_paragraph();

    // Should create a valid paragraph
    EXPECT_TRUE(paragraph.getNode());

    // Body should now contain one paragraph
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& _: paragraph_range)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyTest, AddParagraph_WithText)
{
    const std::string test_text = "Test paragraph content";
    const auto paragraph = m_body->add_paragraph(test_text);

    // Should create a valid paragraph
    EXPECT_TRUE(paragraph.getNode());

    // Verify paragraph was added to body
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& _: paragraph_range)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyTest, AddParagraph_WithFormattingFlag)
{
    const std::string test_text = "Bold text";
    const auto paragraph = m_body->add_paragraph(test_text, duckx::bold);

    // Should create a valid paragraph
    EXPECT_TRUE(paragraph.getNode());

    // Verify paragraph was added
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& _: paragraph_range)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyTest, AddParagraph_MultipleFormattingFlags)
{
    const std::string test_text = "Bold and italic text";
    constexpr duckx::formatting_flag combined_flags = duckx::bold | duckx::italic;
    const auto paragraph = m_body->add_paragraph(test_text, combined_flags);

    // Should create a valid paragraph
    EXPECT_TRUE(paragraph.getNode());
}

TEST_F(BodyTest, AddParagraph_AllFormattingFlags)
{
    const std::string test_text = "All formatting";
    constexpr duckx::formatting_flag all_flags = duckx::bold | duckx::italic | duckx::underline |
                                                 duckx::strikethrough | duckx::superscript |
                                                 duckx::subscript | duckx::smallcaps | duckx::shadow;
    const auto paragraph = m_body->add_paragraph(test_text, all_flags);

    // Should create a valid paragraph
    EXPECT_TRUE(paragraph.getNode());
}

TEST_F(BodyTest, AddParagraph_LongText)
{
    const std::string long_text(10000, 'A');
    const auto paragraph = m_body->add_paragraph(long_text);

    // Should handle long text without issues
    EXPECT_TRUE(paragraph.getNode());
}

TEST_F(BodyTest, AddParagraph_SpecialCharacters)
{
    const std::string special_text = "Special chars: !@#$%^&*()_+-=[]{}|;:,.<>?/~`";
    const auto paragraph = m_body->add_paragraph(special_text);

    EXPECT_TRUE(paragraph.getNode());
}

// Table functionality tests
TEST_F(BodyTest, Tables_EmptyBody)
{
    auto table_range = m_body->tables();

    // Empty body should return empty range
    const auto it = table_range.begin();
    const auto end = table_range.end();
    EXPECT_EQ(it, end);
}

TEST_F(BodyTest, Tables_WithExistingTables)
{
    create_table_node(2, 3);
    create_table_node(1, 2);

    auto table_range = m_body->tables();
    auto it = table_range.begin();
    auto end = table_range.end();

    // Should be able to iterate through tables
    EXPECT_NE(it, end);

    int count = 0;
    for (auto& table: table_range)
    {
        ++count;
        EXPECT_TRUE(table.getNode());
    }
    EXPECT_EQ(count, 2);
}

TEST_F(BodyTest, AddTable_ValidDimensions)
{
    constexpr int rows = 3;
    constexpr int cols = 4;
    const auto table = m_body->add_table(rows, cols);

    // Should create a valid table
    EXPECT_TRUE(table.getNode());

    // Body should now contain one table
    auto table_range = m_body->tables();
    int count = 0;
    for (auto& _: table_range)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyTest, AddTable_MinimumDimensions)
{
    constexpr int rows = 1;
    constexpr int cols = 1;
    const auto table = m_body->add_table(rows, cols);

    // Should create a valid table with minimum dimensions
    EXPECT_TRUE(table.getNode());
}

TEST_F(BodyTest, AddTable_LargeDimensions)
{
    constexpr int rows = 100;
    constexpr int cols = 50;
    const auto table = m_body->add_table(rows, cols);

    // Should handle large tables
    EXPECT_TRUE(table.getNode());
}

TEST_F(BodyTest, AddTable_ZeroRows)
{
    constexpr int rows = 0;
    constexpr int cols = 5;
    auto table = m_body->add_table(rows, cols);

    // Should handle zero rows gracefully
    // Implementation may create empty table or handle as edge case
}

TEST_F(BodyTest, AddTable_ZeroCols)
{
    constexpr int rows = 5;
    constexpr int cols = 0;
    auto table = m_body->add_table(rows, cols);

    // Should handle zero columns gracefully
    // Implementation may create empty table or handle as edge case
}

TEST_F(BodyTest, AddTable_NegativeDimensions)
{
    constexpr int rows = -1;
    constexpr int cols = -1;
    auto table = m_body->add_table(rows, cols);

    // Should handle negative dimensions gracefully
    // Implementation should either throw or handle as edge case
}

// Integration tests
TEST_F(BodyTest, MixedContent_ParagraphsAndTables)
{
    // Add mixed content
    m_body->add_paragraph("First paragraph");
    m_body->add_table(2, 2);
    m_body->add_paragraph("Second paragraph", duckx::bold);
    m_body->add_table(1, 3);

    std::ostringstream oss;
    m_doc->save(oss);

    auto paragraph_range = m_body->paragraphs();
    int para_count = 0;
    for (auto& p: paragraph_range)
    {
        ++para_count;
        EXPECT_TRUE(p.getNode());
    }
    EXPECT_EQ(para_count, 2);

    // 验证表格数
    auto table_range = m_body->tables();
    int table_count = 0;
    for (auto& t: table_range)
    {
        ++table_count;
        EXPECT_TRUE(t.getNode());
    }
    EXPECT_EQ(table_count, 2);
}

TEST_F(BodyTest, SequentialOperations)
{
    // Test sequential operations
    for (int i = 0; i < 5; ++i)
    {
        std::string text = "Paragraph " + std::to_string(i);
        auto paragraph = m_body->add_paragraph(text);
        EXPECT_TRUE(paragraph.getNode());
    }

    for (int i = 0; i < 3; ++i)
    {
        auto table = m_body->add_table(i + 1, i + 2);
        EXPECT_TRUE(table.getNode());
    }

    // Verify final counts
    auto paragraph_range = m_body->paragraphs();
    int para_count = 0;
    for (auto& _: paragraph_range)
    {
        ++para_count;
    }
    EXPECT_EQ(para_count, 5);

    auto table_range = m_body->tables();
    int table_count = 0;
    for (auto& _: table_range)
    {
        ++table_count;
    }
    EXPECT_EQ(table_count, 3);
}

// Error condition tests
TEST_F(BodyTest, OperationsOnInvalidBody)
{
    duckx::Body invalid_body;

    // Operations on invalid body should not crash
    EXPECT_NO_THROW(invalid_body.paragraphs());
    EXPECT_NO_THROW(invalid_body.tables());
    EXPECT_NO_THROW(invalid_body.add_paragraph("test"));
    EXPECT_NO_THROW(invalid_body.add_table(1, 1));
}

TEST_F(BodyTest, RangeIteratorStability)
{
    // Add some content
    m_body->add_paragraph("Para 1");
    m_body->add_paragraph("Para 2");

    auto paragraph_range = m_body->paragraphs();
    auto it1 = paragraph_range.begin();
    auto it2 = paragraph_range.begin();

    // Iterators to same position should be equal
    EXPECT_EQ(it1, it2);

    ++it1;
    EXPECT_NE(it1, it2);

    ++it2;
    EXPECT_EQ(it1, it2);
}
