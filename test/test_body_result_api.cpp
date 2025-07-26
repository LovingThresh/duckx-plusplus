#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Body.hpp"
#include "BaseElement.hpp"
#include "constants.hpp"
#include "Error.hpp"
#include <memory>
#include <string>

class BodyResultAPITest : public ::testing::Test
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

    std::unique_ptr<pugi::xml_document> m_doc;
    pugi::xml_node m_body_node;
    std::unique_ptr<duckx::Body> m_body;
};

// ============================================================================
// add_paragraph_safe() Tests
// ============================================================================

TEST_F(BodyResultAPITest, AddParagraphSafe_Success_EmptyText)
{
    auto result = m_body->add_paragraph_safe();
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
    
    // Verify paragraph was added to body
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& para : paragraph_range)
    {
        ++count;
        EXPECT_TRUE(para.get_node());
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Success_WithText)
{
    const std::string test_text = "Test paragraph content";
    auto result = m_body->add_paragraph_safe(test_text);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
    
    // Verify paragraph was added
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& para : paragraph_range)
    {
        ++count;
        EXPECT_TRUE(para.get_node());
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Success_WithFormattingFlag)
{
    const std::string test_text = "Bold text";
    auto result = m_body->add_paragraph_safe(test_text, duckx::bold);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Success_CombinedFormattingFlags)
{
    const std::string test_text = "Bold and italic text";
    constexpr duckx::formatting_flag combined_flags = duckx::bold | duckx::italic;
    auto result = m_body->add_paragraph_safe(test_text, combined_flags);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Success_SpecialCharacters)
{
    const std::string special_text = "Special chars: !@#$%^&*()_+-=[]{}|;:,.<>?/~`";
    auto result = m_body->add_paragraph_safe(special_text);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Success_UnicodeText)
{
    const std::string unicode_text = "Unicode: 中文 🌟 αβγ ñ";
    auto result = m_body->add_paragraph_safe(unicode_text);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Failure_InvalidBody)
{
    duckx::Body invalid_body; // Default constructor creates invalid body
    
    auto result = invalid_body.add_paragraph_safe("test");
    
    ASSERT_FALSE(result.ok()) << "Expected error for invalid body";
    EXPECT_EQ(result.error().category(), duckx::ErrorCategory::ELEMENT_OPERATION);
    EXPECT_EQ(result.error().code(), duckx::ErrorCode::ELEMENT_INVALID_STATE);
    EXPECT_TRUE(result.error().message().find("Body node is invalid or null") != std::string::npos);
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Failure_ExcessiveTextLength)
{
    const std::string huge_text(2000000, 'A'); // 2MB text, exceeds 1MB limit
    
    auto result = m_body->add_paragraph_safe(huge_text);
    
    ASSERT_FALSE(result.ok()) << "Expected error for excessive text length";
    EXPECT_EQ(result.error().category(), duckx::ErrorCategory::VALIDATION);
    EXPECT_EQ(result.error().code(), duckx::ErrorCode::INVALID_TEXT_LENGTH);
    EXPECT_TRUE(result.error().message().find("Text length exceeds maximum allowed size") != std::string::npos);
    if (result.error().context()) {
        EXPECT_TRUE(result.error().context()->to_string().find("text_length=2000000") != std::string::npos);
    }
}

TEST_F(BodyResultAPITest, AddParagraphSafe_Multiple_Success)
{
    // Add multiple paragraphs
    for (int i = 0; i < 5; ++i)
    {
        std::string text = "Paragraph " + std::to_string(i);
        auto result = m_body->add_paragraph_safe(text);
        ASSERT_TRUE(result.ok()) << "Failed to add paragraph " << i << ": " << result.error().to_string();
        EXPECT_TRUE(result.value().get_node());
    }
    
    // Verify all paragraphs were added
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& para : paragraph_range)
    {
        ++count;
        EXPECT_TRUE(para.get_node());
    }
    EXPECT_EQ(count, 5);
}

// ============================================================================
// add_table_safe() Tests
// ============================================================================

TEST_F(BodyResultAPITest, AddTableSafe_Success_ValidDimensions)
{
    constexpr int rows = 3;
    constexpr int cols = 4;
    auto result = m_body->add_table_safe(rows, cols);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
    
    // Verify table was added to body
    auto table_range = m_body->tables();
    int count = 0;
    for (auto& table : table_range)
    {
        ++count;
        EXPECT_TRUE(table.get_node());
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyResultAPITest, AddTableSafe_Success_MinimumDimensions)
{
    constexpr int rows = 1;
    constexpr int cols = 1;
    auto result = m_body->add_table_safe(rows, cols);
    
    ASSERT_TRUE(result.ok()) << "Expected success, got error: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
}

TEST_F(BodyResultAPITest, AddTableSafe_Success_ZeroDimensions)
{
    // Test with zero rows and columns (should succeed but create empty table)
    auto result_zero_rows = m_body->add_table_safe(0, 3);
    ASSERT_TRUE(result_zero_rows.ok()) << "Expected success for zero rows: " << result_zero_rows.error().to_string();
    
    auto result_zero_cols = m_body->add_table_safe(3, 0);
    ASSERT_TRUE(result_zero_cols.ok()) << "Expected success for zero cols: " << result_zero_cols.error().to_string();
    
    auto result_both_zero = m_body->add_table_safe(0, 0);
    ASSERT_TRUE(result_both_zero.ok()) << "Expected success for both zero: " << result_both_zero.error().to_string();
}

TEST_F(BodyResultAPITest, AddTableSafe_Failure_InvalidBody)
{
    duckx::Body invalid_body; // Default constructor creates invalid body
    
    auto result = invalid_body.add_table_safe(2, 2);
    
    ASSERT_FALSE(result.ok()) << "Expected error for invalid body";
    EXPECT_EQ(result.error().category(), duckx::ErrorCategory::ELEMENT_OPERATION);
    EXPECT_EQ(result.error().code(), duckx::ErrorCode::ELEMENT_INVALID_STATE);
    EXPECT_TRUE(result.error().message().find("Body node is invalid or null") != std::string::npos);
}

TEST_F(BodyResultAPITest, AddTableSafe_Failure_NegativeDimensions)
{
    auto result_negative_rows = m_body->add_table_safe(-1, 3);
    
    ASSERT_FALSE(result_negative_rows.ok()) << "Expected error for negative rows";
    EXPECT_EQ(result_negative_rows.error().category(), duckx::ErrorCategory::VALIDATION);
    EXPECT_EQ(result_negative_rows.error().code(), duckx::ErrorCode::INVALID_ARGUMENT);
    EXPECT_TRUE(result_negative_rows.error().message().find("Table dimensions cannot be negative") != std::string::npos);
    if (result_negative_rows.error().context()) {
        EXPECT_TRUE(result_negative_rows.error().context()->to_string().find("rows=-1") != std::string::npos);
    }
    
    auto result_negative_cols = m_body->add_table_safe(3, -1);
    
    ASSERT_FALSE(result_negative_cols.ok()) << "Expected error for negative cols";
    EXPECT_EQ(result_negative_cols.error().category(), duckx::ErrorCategory::VALIDATION);
    EXPECT_EQ(result_negative_cols.error().code(), duckx::ErrorCode::INVALID_ARGUMENT);
    EXPECT_TRUE(result_negative_cols.error().message().find("Table dimensions cannot be negative") != std::string::npos);
    if (result_negative_cols.error().context()) {
        EXPECT_TRUE(result_negative_cols.error().context()->to_string().find("cols=-1") != std::string::npos);
    }
}

TEST_F(BodyResultAPITest, AddTableSafe_Failure_ExcessiveDimensions)
{
    // Test exceeding row limit
    auto result_too_many_rows = m_body->add_table_safe(15000, 2);
    
    ASSERT_FALSE(result_too_many_rows.ok()) << "Expected error for too many rows";
    EXPECT_EQ(result_too_many_rows.error().category(), duckx::ErrorCategory::VALIDATION);
    EXPECT_EQ(result_too_many_rows.error().code(), duckx::ErrorCode::RESOURCE_LIMIT_EXCEEDED);
    EXPECT_TRUE(result_too_many_rows.error().message().find("Table dimensions exceed maximum allowed size") != std::string::npos);
    if (result_too_many_rows.error().context()) {
        EXPECT_TRUE(result_too_many_rows.error().context()->to_string().find("rows=15000") != std::string::npos);
    }
    
    // Test exceeding column limit
    auto result_too_many_cols = m_body->add_table_safe(2, 1500);
    
    ASSERT_FALSE(result_too_many_cols.ok()) << "Expected error for too many columns";
    EXPECT_EQ(result_too_many_cols.error().category(), duckx::ErrorCategory::VALIDATION);
    EXPECT_EQ(result_too_many_cols.error().code(), duckx::ErrorCode::RESOURCE_LIMIT_EXCEEDED);
    EXPECT_TRUE(result_too_many_cols.error().message().find("Table dimensions exceed maximum allowed size") != std::string::npos);
    if (result_too_many_cols.error().context()) {
        EXPECT_TRUE(result_too_many_cols.error().context()->to_string().find("cols=1500") != std::string::npos);
    }
}

TEST_F(BodyResultAPITest, AddTableSafe_Success_LargeDimensions)
{
    // Test within limits but still large
    constexpr int rows = 100;
    constexpr int cols = 50;
    auto result = m_body->add_table_safe(rows, cols);
    
    ASSERT_TRUE(result.ok()) << "Expected success for large but valid dimensions: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
}

TEST_F(BodyResultAPITest, AddTableSafe_Multiple_Success)
{
    // Add multiple tables with different dimensions
    std::vector<std::pair<int, int>> dimensions = {{2, 3}, {1, 5}, {4, 2}, {3, 3}};
    
    for (size_t i = 0; i < dimensions.size(); ++i)
    {
        int rows = dimensions[i].first;
        int cols = dimensions[i].second;
        auto result = m_body->add_table_safe(rows, cols);
        ASSERT_TRUE(result.ok()) << "Failed to add table " << i << " (" << rows << "x" << cols << "): " 
                                 << result.error().to_string();
        EXPECT_TRUE(result.value().get_node());
    }
    
    // Verify all tables were added
    auto table_range = m_body->tables();
    int count = 0;
    for (auto& table : table_range)
    {
        ++count;
        EXPECT_TRUE(table.get_node());
    }
    EXPECT_EQ(count, dimensions.size());
}

// ============================================================================
// Integration Tests - Mixed Content with Result API
// ============================================================================

TEST_F(BodyResultAPITest, MixedContent_ParagraphsAndTables_ResultAPI)
{
    // Add mixed content using Result API
    auto para1_result = m_body->add_paragraph_safe("First paragraph");
    ASSERT_TRUE(para1_result.ok());
    
    auto table1_result = m_body->add_table_safe(2, 2);
    ASSERT_TRUE(table1_result.ok());
    
    auto para2_result = m_body->add_paragraph_safe("Second paragraph", duckx::bold);
    ASSERT_TRUE(para2_result.ok());
    
    auto table2_result = m_body->add_table_safe(1, 3);
    ASSERT_TRUE(table2_result.ok());
    
    // Verify final counts
    auto paragraph_range = m_body->paragraphs();
    int para_count = 0;
    for (auto& para : paragraph_range)
    {
        ++para_count;
        EXPECT_TRUE(para.get_node());
    }
    EXPECT_EQ(para_count, 2);
    
    auto table_range = m_body->tables();
    int table_count = 0;
    for (auto& table : table_range)
    {
        ++table_count;
        EXPECT_TRUE(table.get_node());
    }
    EXPECT_EQ(table_count, 2);
}

// ============================================================================
// Error Context and Information Tests
// ============================================================================

TEST_F(BodyResultAPITest, ErrorContext_Contains_Function_And_File_Info)
{
    duckx::Body invalid_body;
    
    auto result = invalid_body.add_paragraph_safe("test");
    
    ASSERT_FALSE(result.ok());
    if (result.error().context()) {
        const auto& context = *result.error().context();
        
        EXPECT_TRUE(context.function_name.find("add_paragraph_safe") != std::string::npos);
        EXPECT_TRUE(context.file_path.find("Body.cpp") != std::string::npos);
        EXPECT_GT(context.line_number, 0);
        EXPECT_TRUE(context.operation.find("add_paragraph_safe") != std::string::npos);
    }
}

TEST_F(BodyResultAPITest, ErrorContext_Contains_Additional_Info)
{
    const std::string huge_text(2000000, 'A');
    
    auto result = m_body->add_paragraph_safe(huge_text);
    
    ASSERT_FALSE(result.ok());
    if (result.error().context()) {
        const auto& context = *result.error().context();
        
        EXPECT_EQ(context.additional_info.count("text_length"), 1);
        EXPECT_EQ(context.additional_info.count("max_length"), 1);
        EXPECT_EQ(context.additional_info.at("text_length"), "2000000");
        EXPECT_EQ(context.additional_info.at("max_length"), "1000000");
    }
}

// ============================================================================
// Monadic Operations Tests (and_then, or_else)
// ============================================================================

TEST_F(BodyResultAPITest, MonadicOperations_AndThen_Success_Chain)
{
    auto para_result = m_body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok()) << "Expected success in paragraph creation: " << para_result.error().to_string();
    
    auto table_result = m_body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok()) << "Expected success in table creation: " << table_result.error().to_string();
    EXPECT_TRUE(table_result.value().get_node());
}

TEST_F(BodyResultAPITest, MonadicOperations_AndThen_Failure_Propagation)
{
    duckx::Body invalid_body;
    
    auto result = invalid_body.add_paragraph_safe("Test");
    // Since invalid_body is not initialized, this should fail
    
    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.error().code(), duckx::ErrorCode::ELEMENT_INVALID_STATE);
}

TEST_F(BodyResultAPITest, MonadicOperations_OrElse_Success_NoFallback)
{
    auto result = m_body->add_paragraph_safe("Test paragraph")
        .or_else([this](const duckx::Error& error) {
            return m_body->add_paragraph_safe("Fallback paragraph");
        });
    
    ASSERT_TRUE(result.ok());
    EXPECT_TRUE(result.value().get_node());
    
    // Should only have one paragraph (the original, not the fallback)
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& para : paragraph_range)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(BodyResultAPITest, MonadicOperations_OrElse_Failure_UseFallback)
{
    duckx::Body invalid_body;
    
    auto result = invalid_body.add_paragraph_safe("Test paragraph")
        .or_else([this](const duckx::Error& error) {
            return m_body->add_paragraph_safe("Fallback paragraph");
        });
    
    ASSERT_TRUE(result.ok()) << "Expected fallback to succeed: " << result.error().to_string();
    EXPECT_TRUE(result.value().get_node());
    
    // Should have one paragraph (the fallback)
    auto paragraph_range = m_body->paragraphs();
    int count = 0;
    for (auto& para : paragraph_range)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}