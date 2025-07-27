/*!
 * @file test_result_api.cpp
 * @brief Unit tests for Result<T> API and error handling system
 * 
 * Tests modern Result<T> error handling patterns, monadic operations,
 * error chaining, and comprehensive safety mechanisms across all
 * library components with detailed error context validation.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include "Document.hpp"
#include "BaseElement.hpp"
#include "Error.hpp"

using namespace duckx;

class ResultAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test environment
    }

    void TearDown() override {
        // Cleanup
    }
};

// ============================================================================
// Document Result<T> API Tests
// ============================================================================

TEST_F(ResultAPITest, DocumentCreateSafeSuccess) {
    auto result = Document::create_safe("test_result_api.docx");
    EXPECT_TRUE(result.ok());
    EXPECT_FALSE(result.has_error());
    
    // Should be able to access the document
    auto& doc = result.value();
    EXPECT_NO_THROW(doc.body());
}

TEST_F(ResultAPITest, DocumentCreateSafeEmptyPath) {
    auto result = Document::create_safe("");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.has_error());
    
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::GENERAL, error.category());
    EXPECT_EQ(ErrorCode::INVALID_ARGUMENT, error.code());
    EXPECT_FALSE(error.message().empty());
}

TEST_F(ResultAPITest, DocumentOpenSafeNonExistentFile) {
    auto result = Document::open_safe("non_existent_file_12345.docx");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.has_error());
    
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::FILE_IO, error.category());
    EXPECT_FALSE(error.message().empty());
}

TEST_F(ResultAPITest, DocumentSaveSafe) {
    auto doc_result = Document::create_safe("test_save_safe.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto save_result = doc_result.value().save_safe();
    EXPECT_TRUE(save_result.ok());
}

// ============================================================================
// Table Result<T> API Tests
// ============================================================================

TEST_F(ResultAPITest, TableSetWidthSafeValid) {
    auto doc_result = Document::create_safe("test_table_width.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto result = table.set_width_safe(100.0);
    
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.value(), &table); // Should return reference to same object
}

TEST_F(ResultAPITest, TableSetWidthSafeNegative) {
    auto doc_result = Document::create_safe("test_table_width_neg.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto result = table.set_width_safe(-50.0);
    
    EXPECT_FALSE(result.ok());
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::GENERAL, error.category());
    EXPECT_EQ(ErrorCode::INVALID_ARGUMENT, error.code());
}

TEST_F(ResultAPITest, TableSetWidthSafeTooLarge) {
    auto doc_result = Document::create_safe("test_table_width_large.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto result = table.set_width_safe(3000.0); // Exceeds maximum
    
    EXPECT_FALSE(result.ok());
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::VALIDATION, error.category());
    EXPECT_EQ(ErrorCode::VALIDATION_FAILED, error.code());
}

TEST_F(ResultAPITest, TableSetAlignmentSafeValid) {
    auto doc_result = Document::create_safe("test_table_align.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    
    auto result1 = table.set_alignment_safe("left");
    EXPECT_TRUE(result1.ok());
    
    auto result2 = table.set_alignment_safe("center");
    EXPECT_TRUE(result2.ok());
    
    auto result3 = table.set_alignment_safe("right");
    EXPECT_TRUE(result3.ok());
}

TEST_F(ResultAPITest, TableSetAlignmentSafeInvalid) {
    auto doc_result = Document::create_safe("test_table_align_invalid.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto result = table.set_alignment_safe("invalid_alignment");
    
    EXPECT_FALSE(result.ok());
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::VALIDATION, error.category());
    EXPECT_EQ(ErrorCode::VALIDATION_FAILED, error.code());
}

TEST_F(ResultAPITest, TableChainedOperations) {
    auto doc_result = Document::create_safe("test_table_chain.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    
    auto result = table.set_width_safe(200.0);
    if (result.ok()) {
        result = table.set_alignment_safe("center");
    }
    if (result.ok()) {
        result = table.set_border_style_safe("single");
    }
    if (result.ok()) {
        result = table.set_border_width_safe(1.0);
    }
    
    EXPECT_TRUE(result.ok());
}

TEST_F(ResultAPITest, TableChainedOperationsWithError) {
    auto doc_result = Document::create_safe("test_table_chain_error.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    
    auto result = table.set_width_safe(200.0);
    if (result.ok()) {
        result = table.set_alignment_safe("invalid"); // This should fail
    }
    if (result.ok()) {
        result = table.set_border_style_safe("single"); // This shouldn't execute
    }
    
    EXPECT_FALSE(result.ok());
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::VALIDATION, error.category());
}

// ============================================================================
// TableCell Result<T> API Tests
// ============================================================================

TEST_F(ResultAPITest, TableCellSetWidthSafeValid) {
    auto doc_result = Document::create_safe("test_cell_width.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    auto cell_iter = row.cells().begin();
    ASSERT_NE(cell_iter, row.cells().end());
    auto& cell = *cell_iter;
    
    auto result = cell.set_width_safe(50.0);
    EXPECT_TRUE(result.ok());
}

TEST_F(ResultAPITest, TableCellSetBackgroundColorSafeValid) {
    auto doc_result = Document::create_safe("test_cell_bg.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    auto cell_iter = row.cells().begin();
    ASSERT_NE(cell_iter, row.cells().end());
    auto& cell = *cell_iter;
    
    auto result = cell.set_background_color_safe("FF0000");
    EXPECT_TRUE(result.ok());
}

TEST_F(ResultAPITest, TableCellSetBackgroundColorSafeInvalid) {
    auto doc_result = Document::create_safe("test_cell_bg_invalid.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    auto cell_iter = row.cells().begin();
    ASSERT_NE(cell_iter, row.cells().end());
    auto& cell = *cell_iter;
    
    auto result = cell.set_background_color_safe("INVALID_COLOR");
    EXPECT_FALSE(result.ok());
    
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::VALIDATION, error.category());
    EXPECT_EQ(ErrorCode::VALIDATION_FAILED, error.code());
}

TEST_F(ResultAPITest, TableCellSetVerticalAlignmentSafeValid) {
    auto doc_result = Document::create_safe("test_cell_valign.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    auto cell_iter = row.cells().begin();
    ASSERT_NE(cell_iter, row.cells().end());
    auto& cell = *cell_iter;
    
    auto result1 = cell.set_vertical_alignment_safe("top");
    EXPECT_TRUE(result1.ok());
    
    auto result2 = cell.set_vertical_alignment_safe("center");
    EXPECT_TRUE(result2.ok());
    
    auto result3 = cell.set_vertical_alignment_safe("bottom");
    EXPECT_TRUE(result3.ok());
}

TEST_F(ResultAPITest, TableCellSetVerticalAlignmentSafeInvalid) {
    auto doc_result = Document::create_safe("test_cell_valign_invalid.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    auto cell_iter = row.cells().begin();
    ASSERT_NE(cell_iter, row.cells().end());
    auto& cell = *cell_iter;
    
    auto result = cell.set_vertical_alignment_safe("invalid_alignment");
    EXPECT_FALSE(result.ok());
    
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::VALIDATION, error.category());
    EXPECT_EQ(ErrorCode::VALIDATION_FAILED, error.code());
}

// ============================================================================
// TableRow Result<T> API Tests
// ============================================================================

TEST_F(ResultAPITest, TableRowSetHeightSafeValid) {
    auto doc_result = Document::create_safe("test_row_height.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    
    auto result = row.set_height_safe(25.0);
    EXPECT_TRUE(result.ok());
}

TEST_F(ResultAPITest, TableRowSetHeightSafeNegative) {
    auto doc_result = Document::create_safe("test_row_height_neg.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    
    auto result = row.set_height_safe(-10.0);
    EXPECT_FALSE(result.ok());
    
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::GENERAL, error.category());
    EXPECT_EQ(ErrorCode::INVALID_ARGUMENT, error.code());
}

TEST_F(ResultAPITest, TableRowSetHeightRuleSafeValid) {
    auto doc_result = Document::create_safe("test_row_height_rule.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    
    auto result1 = row.set_height_rule_safe("exact");
    EXPECT_TRUE(result1.ok());
    
    auto result2 = row.set_height_rule_safe("atLeast");
    EXPECT_TRUE(result2.ok());
    
    auto result3 = row.set_height_rule_safe("auto");
    EXPECT_TRUE(result3.ok());
}

TEST_F(ResultAPITest, TableRowSetHeightRuleSafeInvalid) {
    auto doc_result = Document::create_safe("test_row_height_rule_invalid.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& row = *row_iter;
    
    auto result = row.set_height_rule_safe("invalid_rule");
    EXPECT_FALSE(result.ok());
    
    const auto& error = result.error();
    EXPECT_EQ(ErrorCategory::VALIDATION, error.category());
    EXPECT_EQ(ErrorCode::VALIDATION_FAILED, error.code());
}

// ============================================================================
// Error Context and Information Tests
// ============================================================================

TEST_F(ResultAPITest, ErrorContextInformation) {
    auto doc_result = Document::create_safe("test_error_context.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto table = doc_result.value().body().add_table(2, 2);
    auto result = table.set_width_safe(-100.0); // Invalid width
    
    ASSERT_FALSE(result.ok());
    const auto& error = result.error();
    
    // Check error has context
    EXPECT_TRUE(error.context().has_value());
    
    // Check error string contains useful information
    std::string error_str = error.to_string();
    EXPECT_FALSE(error_str.empty());
    EXPECT_NE(error_str.find("width_pts"), std::string::npos);
    EXPECT_NE(error_str.find("-100"), std::string::npos);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(ResultAPITest, CompleteTableFormattingWorkflow) {
    auto doc_result = Document::create_safe("test_complete_workflow.docx");
    ASSERT_TRUE(doc_result.ok());
    
    auto& doc = doc_result.value();
    auto table = doc.body().add_table(2, 3);
    
    // Configure table
    auto width_result = table.set_width_safe(300.0);
    ASSERT_TRUE(width_result.ok());
    
    auto align_result = table.set_alignment_safe("center");
    ASSERT_TRUE(align_result.ok());
    
    auto border_result = table.set_border_style_safe("single");
    ASSERT_TRUE(border_result.ok());
    
    // Configure first row as header
    auto row_iter = table.rows().begin();
    ASSERT_NE(row_iter, table.rows().end());
    auto& first_row = *row_iter;
    auto height_result = first_row.set_height_safe(30.0);
    ASSERT_TRUE(height_result.ok());
    
    auto header_result = first_row.set_header_row_safe(true);
    ASSERT_TRUE(header_result.ok());
    
    // Configure first cell
    auto cell_iter = first_row.cells().begin();
    ASSERT_NE(cell_iter, first_row.cells().end());
    auto& first_cell = *cell_iter;
    auto bg_result = first_cell.set_background_color_safe("E0E0E0");
    ASSERT_TRUE(bg_result.ok());
    
    auto cell_align_result = first_cell.set_vertical_alignment_safe("center");
    ASSERT_TRUE(cell_align_result.ok());
    
    // Save document
    auto save_result = doc.save_safe();
    EXPECT_TRUE(save_result.ok());
}
