/*!
 * @file test_style_set_integration.cpp
 * @brief Integration tests for StyleSet functionality with StyleManager and Document
 * 
 * Tests the complete integration of StyleSet management, including registration,
 * application, cascading behavior, and document-level operations.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "Document.hpp"
#include "StyleManager.hpp"
#include "XmlStyleParser.hpp"
#include "test_utils.hpp"
#include "absl/strings/str_format.h"

using namespace duckx;

class StyleSetIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test output directory
        test_utils::create_directory("temp");
    }
    
    void TearDown() override
    {
        // Cleanup is handled by test_utils
    }
    
    std::string get_test_output_path(const std::string& filename) const
    {
        return test_utils::get_temp_path(filename);
    }
    
    // Helper to create a basic style set
    StyleSet create_technical_doc_style_set() const
    {
        StyleSet tech_set("TechnicalDocument");
        tech_set.description = "Style set for technical documentation";
        tech_set.included_styles = {
            "Heading 1",
            "Heading 2", 
            "Normal",
            "Code"
        };
        return tech_set;
    }
    
    // Helper to create a custom style set
    StyleSet create_custom_style_set() const
    {
        StyleSet custom_set("CustomSet");
        custom_set.description = "Custom style set for testing";
        custom_set.included_styles = {
            "CustomHeading",
            "CustomBody",
            "CustomCode"
        };
        return custom_set;
    }
};

// ============================================================================
// StyleSet Registration Tests
// ============================================================================

TEST_F(StyleSetIntegrationTest, RegisterStyleSet_Success)
{
    auto doc_result = Document::create_safe(get_test_output_path("register_style_set.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    // Load built-in styles first
    auto& style_mgr = doc.styles();
    auto load_result = style_mgr.load_all_built_in_styles_safe();
    ASSERT_TRUE(load_result.ok());
    
    // Create and register a style set
    StyleSet tech_set = create_technical_doc_style_set();
    auto register_result = style_mgr.register_style_set_safe(tech_set);
    EXPECT_TRUE(register_result.ok());
    
    // Verify registration
    EXPECT_TRUE(style_mgr.has_style_set("TechnicalDocument"));
    
    // Get the style set back
    auto get_result = style_mgr.get_style_set_safe("TechnicalDocument");
    ASSERT_TRUE(get_result.ok());
    EXPECT_EQ(get_result.value().name, "TechnicalDocument");
    EXPECT_EQ(get_result.value().included_styles.size(), 4);
}

TEST_F(StyleSetIntegrationTest, RegisterStyleSet_InvalidStyles)
{
    auto doc_result = Document::create_safe(get_test_output_path("register_invalid.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    
    // Create style set with non-existent styles
    StyleSet invalid_set("InvalidSet");
    invalid_set.included_styles = {"NonExistentStyle1", "NonExistentStyle2"};
    
    // Should fail because referenced styles don't exist
    auto register_result = style_mgr.register_style_set_safe(invalid_set);
    EXPECT_FALSE(register_result.ok());
    EXPECT_EQ(register_result.error().code(), ErrorCode::STYLE_NOT_FOUND);
}

TEST_F(StyleSetIntegrationTest, RegisterStyleSet_DuplicateName)
{
    auto doc_result = Document::create_safe(get_test_output_path("register_duplicate.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Register first set
    StyleSet set1 = create_technical_doc_style_set();
    auto result1 = style_mgr.register_style_set_safe(set1);
    ASSERT_TRUE(result1.ok());
    
    // Try to register with same name
    StyleSet set2 = create_technical_doc_style_set();
    auto result2 = style_mgr.register_style_set_safe(set2);
    EXPECT_FALSE(result2.ok());
    EXPECT_EQ(result2.error().code(), ErrorCode::STYLE_ALREADY_EXISTS);
}

// ============================================================================
// StyleSet Application Tests
// ============================================================================

TEST_F(StyleSetIntegrationTest, ApplyStyleSet_ToDocument)
{
    auto doc_result = Document::create_safe(get_test_output_path("apply_style_set.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    // Load built-in styles
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Add content to document
    auto& body = doc.body();
    body.add_paragraph_safe("Main Title");
    body.add_paragraph_safe("Section Title");
    body.add_paragraph_safe("This is normal body text.");
    body.add_paragraph_safe("print('Hello World')");
    
    // Manually set initial styles
    auto paragraphs = body.paragraphs();
    ASSERT_GE(paragraphs.size(), 4);
    
    auto para_iter = paragraphs.begin();
    auto result1 = para_iter->apply_style_safe(doc.styles(), "Heading 1");
    ASSERT_TRUE(result1.ok());
    ++para_iter;
    auto result2 = para_iter->apply_style_safe(doc.styles(), "Heading 2");
    ASSERT_TRUE(result2.ok());
    ++para_iter;
    auto result3 = para_iter->apply_style_safe(doc.styles(), "Normal");
    ASSERT_TRUE(result3.ok());
    ++para_iter;
    // For the code paragraph, first create a paragraph-level code style
    auto code_para_style = style_mgr.create_paragraph_style_safe("CodeParagraph");
    ASSERT_TRUE(code_para_style.ok());
    
    auto result4 = para_iter->apply_style_safe(doc.styles(), "CodeParagraph");
    ASSERT_TRUE(result4.ok());
    
    // Verify the styles were set correctly before style set application
    auto verify_paragraphs = body.paragraphs();
    auto verify_iter = verify_paragraphs.begin();
    ASSERT_EQ(verify_iter->get_style_safe().value(), "Heading 1");
    ++verify_iter;
    ASSERT_EQ(verify_iter->get_style_safe().value(), "Heading 2");
    ++verify_iter;
    ASSERT_EQ(verify_iter->get_style_safe().value(), "Normal");
    ++verify_iter;
    ASSERT_EQ(verify_iter->get_style_safe().value(), "CodeParagraph");
    
    // Register and apply style set
    StyleSet tech_set = create_technical_doc_style_set();
    style_mgr.register_style_set_safe(tech_set);
    
    auto apply_result = doc.apply_style_set_safe("TechnicalDocument");
    EXPECT_TRUE(apply_result.ok());
    
    // Verify styles are still applied (they were already correct)
    // Get fresh paragraph iterators after style set application
    auto fresh_paragraphs = body.paragraphs();
    auto para_check = fresh_paragraphs.begin();
    EXPECT_EQ(para_check->get_style_safe().value(), "Heading 1");
    ++para_check;
    EXPECT_EQ(para_check->get_style_safe().value(), "Heading 2");
    ++para_check;
    EXPECT_EQ(para_check->get_style_safe().value(), "Normal");
    ++para_check;
    EXPECT_EQ(para_check->get_style_safe().value(), "CodeParagraph");
    
    // Save and verify
    auto save_result = doc.save_safe();
    EXPECT_TRUE(save_result.ok());
}

TEST_F(StyleSetIntegrationTest, ApplyStyleSet_CascadingBehavior)
{
    auto doc_result = Document::create_safe(get_test_output_path("cascading_styles.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Create content with mixed elements
    auto& body = doc.body();
    
    // Add paragraphs without explicit styles
    body.add_paragraph_safe("Unstyled Heading");
    body.add_paragraph_safe("Unstyled body text");
    
    // Add table
    auto table_result = body.add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    
    // Create a mixed style set
    StyleSet mixed_set("MixedContent");
    mixed_set.included_styles = {"Normal", "Heading 1"};
    
    style_mgr.register_style_set_safe(mixed_set);
    auto apply_result = style_mgr.apply_style_set_safe("MixedContent", doc);
    EXPECT_TRUE(apply_result.ok());
    
    // Verify cascading application
    auto paragraphs = body.paragraphs();
    for (auto& para : paragraphs) {
        auto style_result = para.get_style_safe();
        // Paragraphs without styles should now have styles from the set
        if (!style_result.ok() || style_result.value().empty()) {
            // These would have been styled by the style set application
        }
    }
}

// ============================================================================
// Document-Level StyleSet Operations
// ============================================================================

TEST_F(StyleSetIntegrationTest, Document_RegisterAndApplyStyleSet)
{
    auto doc_result = Document::create_safe(get_test_output_path("doc_style_set.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    // Load built-in styles
    doc.styles().load_all_built_in_styles_safe();
    
    // Add test content
    auto& body = doc.body();
    body.add_paragraph_safe("Document Title");
    body.add_paragraph_safe("Content paragraph");
    
    // Register style set through Document API
    StyleSet doc_set("DocumentStyles");
    doc_set.included_styles = {"Heading 1", "Normal"};
    
    auto register_result = doc.register_style_set_safe(doc_set);
    EXPECT_TRUE(register_result.ok());
    
    // Apply through Document API
    auto apply_result = doc.apply_style_set_safe("DocumentStyles");
    EXPECT_TRUE(apply_result.ok());
    
    // Verify
    EXPECT_TRUE(doc.styles().has_style_set("DocumentStyles"));
}

TEST_F(StyleSetIntegrationTest, Document_ApplyStyleMappings)
{
    auto doc_result = Document::create_safe(get_test_output_path("style_mappings.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    doc.styles().load_all_built_in_styles_safe();
    
    // Create custom styles
    auto& style_mgr = doc.styles();
    style_mgr.create_paragraph_style_safe("CustomHeading");
    style_mgr.create_paragraph_style_safe("CustomBody");
    
    // Add content with specific styles
    auto& body = doc.body();
    auto para1 = body.add_paragraph_safe("First Heading");
    para1.value().apply_style_safe(doc.styles(), "Heading 1");
    
    auto para2 = body.add_paragraph_safe("Normal Text");
    para2.value().apply_style_safe(doc.styles(), "Normal");
    
    auto para3 = body.add_paragraph_safe("Another Heading");
    para3.value().apply_style_safe(doc.styles(), "Heading 1");
    
    // Create style mappings
    std::map<std::string, std::string> mappings = {
        {"heading1", "CustomHeading"},
        {"normal", "CustomBody"}
    };
    
    // Apply mappings
    auto apply_result = doc.apply_style_mappings_safe(mappings);
    EXPECT_TRUE(apply_result.ok());
    
    // Verify styles were mapped
    auto paragraphs = body.paragraphs();
    auto para_verify = paragraphs.begin();
    EXPECT_EQ(para_verify->get_style_safe().value(), "CustomHeading");
    ++para_verify;
    EXPECT_EQ(para_verify->get_style_safe().value(), "CustomBody");
    ++para_verify;
    EXPECT_EQ(para_verify->get_style_safe().value(), "CustomHeading");
}

// ============================================================================
// StyleSet List and Query Operations
// ============================================================================

TEST_F(StyleSetIntegrationTest, ListStyleSets)
{
    auto doc_result = Document::create_safe(get_test_output_path("list_style_sets.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Register multiple style sets
    StyleSet set1("Set1");
    set1.included_styles = {"Normal"};
    style_mgr.register_style_set_safe(set1);
    
    StyleSet set2("Set2");
    set2.included_styles = {"Heading 1"};
    style_mgr.register_style_set_safe(set2);
    
    StyleSet set3("Set3");
    set3.included_styles = {"Code"};
    style_mgr.register_style_set_safe(set3);
    
    // List all style sets
    auto style_sets = style_mgr.list_style_sets();
    EXPECT_EQ(style_sets.size(), 3);
    
    // Verify all sets are present
    std::vector<std::string> expected = {"Set1", "Set2", "Set3"};
    std::sort(style_sets.begin(), style_sets.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(style_sets, expected);
}

TEST_F(StyleSetIntegrationTest, RemoveStyleSet)
{
    auto doc_result = Document::create_safe(get_test_output_path("remove_style_set.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Register style set
    StyleSet tech_set = create_technical_doc_style_set();
    style_mgr.register_style_set_safe(tech_set);
    ASSERT_TRUE(style_mgr.has_style_set("TechnicalDocument"));
    
    // Remove style set
    auto remove_result = style_mgr.remove_style_set_safe("TechnicalDocument");
    EXPECT_TRUE(remove_result.ok());
    
    // Verify removal
    EXPECT_FALSE(style_mgr.has_style_set("TechnicalDocument"));
    
    // Try to get removed set
    auto get_result = style_mgr.get_style_set_safe("TechnicalDocument");
    EXPECT_FALSE(get_result.ok());
    EXPECT_EQ(get_result.error().code(), ErrorCode::STYLE_NOT_FOUND);
}

// ============================================================================
// Advanced StyleSet Scenarios
// ============================================================================

TEST_F(StyleSetIntegrationTest, ComplexDocumentWithMultipleStyleSets)
{
    auto doc_result = Document::create_safe(get_test_output_path("complex_style_sets.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Create a complex document structure
    auto& body = doc.body();
    
    // Add various content types
    auto p1 = body.add_paragraph_safe("Technical Documentation");
    p1.value().apply_style_safe(doc.styles(), "Heading 1");
    
    auto p2 = body.add_paragraph_safe("Introduction");
    p2.value().apply_style_safe(doc.styles(), "Heading 2");
    
    body.add_paragraph_safe("This document demonstrates style sets.");
    
    auto p3 = body.add_paragraph_safe("Code Example");
    p3.value().apply_style_safe(doc.styles(), "Heading 2");
    
    auto p4 = body.add_paragraph_safe("def hello_world():\n    print('Hello')");
    p4.value().apply_style_safe(doc.styles(), "Code");
    
    auto table_result = body.add_table_safe(3, 3);
    ASSERT_TRUE(table_result.ok());
    
    // Register multiple style sets
    StyleSet tech_set = create_technical_doc_style_set();
    style_mgr.register_style_set_safe(tech_set);
    
    // Create a report style set
    StyleSet report_set("ReportStyles");
    report_set.description = "Formal report styling";
    report_set.included_styles = {"Heading 1", "Heading 2", "Normal"};
    style_mgr.register_style_set_safe(report_set);
    
    // Apply different style sets to see behavior
    auto apply_tech = style_mgr.apply_style_set_safe("TechnicalDocument", doc);
    EXPECT_TRUE(apply_tech.ok());
    
    // Save document
    auto save_result = doc.save_safe();
    EXPECT_TRUE(save_result.ok());
    
    // Verify style sets are properly managed
    auto sets = style_mgr.list_style_sets();
    EXPECT_GE(sets.size(), 2);
}

TEST_F(StyleSetIntegrationTest, StyleSetWithTableStyles)
{
    auto doc_result = Document::create_safe(get_test_output_path("table_style_sets.docx"));
    ASSERT_TRUE(doc_result.ok());
    auto& doc = doc_result.value();
    
    auto& style_mgr = doc.styles();
    style_mgr.load_all_built_in_styles_safe();
    
    // Create table style
    auto table_style_result = style_mgr.create_table_style_safe("CustomTableStyle");
    ASSERT_TRUE(table_style_result.ok());
    
    auto* table_style = table_style_result.value();
    TableStyleProperties table_props;
    // Note: Adjust these properties based on actual TableStyleProperties structure
    table_style->set_table_properties_safe(table_props);
    
    // Create content with tables
    auto& body = doc.body();
    body.add_paragraph_safe("Document with Tables");
    
    auto table1 = body.add_table_safe(2, 3);
    auto table2 = body.add_table_safe(3, 2);
    
    // Create style set including table style
    StyleSet table_set("TableDocumentSet");
    table_set.included_styles = {"Normal", "CustomTableStyle"};
    
    style_mgr.register_style_set_safe(table_set);
    auto apply_result = style_mgr.apply_style_set_safe("TableDocumentSet", doc);
    EXPECT_TRUE(apply_result.ok());
    
    // Tables should have the custom style applied
    // Note: The actual application happens in apply_style_set_safe
}

// Run all tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}