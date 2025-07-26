/*!
 * @file test_style_application.cpp
 * @brief Unit tests for style application functionality
 * 
 * Tests the integration between StyleManager and document elements,
 * verifying that styles can be properly applied to paragraphs, runs,
 * and tables with correct XML output.
 * 
 * @author DuckX-PLusPlus Development Team
 * @date 2025
 */

#include <gtest/gtest.h>
#include <pugixml.hpp>
#include <cstdio>

#include "duckx.hpp"
#include "test_utils.hpp"

using namespace duckx;

namespace duckx {
namespace test {

class StyleApplicationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a temporary document for testing in current directory
        test_path = "style_application_test.docx";
        
        auto doc_result = duckx::Document::create_safe(test_path);
        ASSERT_TRUE(doc_result.ok()) << "Failed to create test document: " << doc_result.error().to_string();
        
        doc = std::make_unique<duckx::Document>(std::move(doc_result.value()));
        style_manager = &doc->styles();
        body = &doc->body();

        // Load built-in styles for testing
        auto builtin_result = style_manager->load_all_built_in_styles_safe();
        ASSERT_TRUE(builtin_result.ok()) << "Failed to load built-in styles: " << builtin_result.error().to_string();
    }

    void TearDown() override
    {
        doc.reset();
        
        // Clean up test file
        std::remove(test_path.c_str());
    }

    std::string test_path;
    std::unique_ptr<duckx::Document> doc;
    duckx::StyleManager* style_manager;
    duckx::Body* body;
};

// ============================================================================
// Paragraph Style Application Tests
// ============================================================================

TEST_F(StyleApplicationTest, ApplyBuiltinParagraphStyle)
{
    // Add a paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    // Apply Heading 1 style
    auto apply_result = para->apply_style_safe(*style_manager, "Heading 1");
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply Heading 1 style: " << apply_result.error().to_string();

    // Verify style was applied
    auto get_style_result = para->get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_EQ("Heading 1", get_style_result.value());
}

TEST_F(StyleApplicationTest, ApplyCustomParagraphStyle)
{
    // Create custom paragraph style
    auto style_result = style_manager->create_mixed_style_safe("Custom Para");
    ASSERT_TRUE(style_result.ok());
    
    duckx::Style* style = style_result.value();
    duckx::ParagraphStyleProperties props;
    props.alignment = duckx::Alignment::CENTER;
    props.space_after_pts = 12.0;
    auto props_result = style->set_paragraph_properties_safe(props);
    ASSERT_TRUE(props_result.ok());

    // Add a paragraph and apply style
    auto para_result = body->add_paragraph_safe("Custom styled paragraph");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    auto apply_result = para->apply_style_safe(*style_manager, "Custom Para");
    EXPECT_TRUE(apply_result.ok());

    // Verify style was applied
    auto get_style_result = para->get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_EQ("Custom Para", get_style_result.value());
}

TEST_F(StyleApplicationTest, ApplyIncompatibleStyleToParagraph)
{
    // Create character-only style
    auto style_result = style_manager->create_character_style_safe("Char Only");
    ASSERT_TRUE(style_result.ok());

    // Add a paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    // Try to apply character style to paragraph (should fail)
    auto apply_result = para->apply_style_safe(*style_manager, "Char Only");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(duckx::ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

TEST_F(StyleApplicationTest, RemoveParagraphStyle)
{
    // Add paragraph and apply style
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    auto apply_result = para->apply_style_safe(*style_manager, "Normal");
    ASSERT_TRUE(apply_result.ok());

    // Remove style
    auto remove_result = para->remove_style_safe();
    EXPECT_TRUE(remove_result.ok());

    // Verify style was removed
    auto get_style_result = para->get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_TRUE(get_style_result.value().empty());
}

// ============================================================================
// Character Style Application Tests
// ============================================================================

TEST_F(StyleApplicationTest, ApplyBuiltinCharacterStyle)
{
    // Add a paragraph and run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    duckx::Run& run = para->add_run("Code text");

    // Apply Code style
    auto apply_result = run.apply_style_safe(*style_manager, "Code");
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply Code style: " << apply_result.error().to_string();

    // Verify style was applied
    auto get_style_result = run.get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_EQ("Code", get_style_result.value());
}

TEST_F(StyleApplicationTest, ApplyCustomCharacterStyle)
{
    // Create custom character style
    auto style_result = style_manager->create_character_style_safe("Custom Char");
    ASSERT_TRUE(style_result.ok());
    
    duckx::Style* style = style_result.value();
    duckx::CharacterStyleProperties props;
    props.font_name = "Times New Roman";
    props.font_size_pts = 14.0;
    props.font_color_hex = "FF0000";
    auto props_result = style->set_character_properties_safe(props);
    ASSERT_TRUE(props_result.ok());

    // Add a paragraph and run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    duckx::Run& run = para->add_run("Custom styled text");

    // Apply custom character style
    auto apply_result = run.apply_style_safe(*style_manager, "Custom Char");
    EXPECT_TRUE(apply_result.ok());

    // Verify style was applied
    auto get_style_result = run.get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_EQ("Custom Char", get_style_result.value());
}

TEST_F(StyleApplicationTest, ApplyMixedStyleToRun)
{
    // Create mixed style
    auto style_result = style_manager->create_mixed_style_safe("Mixed Style");
    ASSERT_TRUE(style_result.ok());
    
    duckx::Style* style = style_result.value();
    duckx::CharacterStyleProperties char_props;
    char_props.font_name = "Arial";
    char_props.font_size_pts = 12.0;
    auto props_result = style->set_character_properties_safe(char_props);
    ASSERT_TRUE(props_result.ok());

    // Add a paragraph and run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    duckx::Run& run = para->add_run("Mixed style text");

    // Apply mixed style to run (should work for character properties)
    auto apply_result = run.apply_style_safe(*style_manager, "Mixed Style");
    EXPECT_TRUE(apply_result.ok());

    // Verify style was applied
    auto get_style_result = run.get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_EQ("Mixed Style", get_style_result.value());
}

TEST_F(StyleApplicationTest, ApplyIncompatibleStyleToRun)
{
    // Create paragraph-only style
    auto style_result = style_manager->create_paragraph_style_safe("Para Only");
    ASSERT_TRUE(style_result.ok());

    // Add a paragraph and run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    duckx::Run& run = para->add_run("Test text");

    // Try to apply paragraph style to run (should fail)
    auto apply_result = run.apply_style_safe(*style_manager, "Para Only");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(duckx::ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

TEST_F(StyleApplicationTest, RemoveCharacterStyle)
{
    // Add run and apply style
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    duckx::Run& run = para->add_run("Test text");

    auto apply_result = run.apply_style_safe(*style_manager, "Code");
    ASSERT_TRUE(apply_result.ok());

    // Remove style
    auto remove_result = run.remove_style_safe();
    EXPECT_TRUE(remove_result.ok());

    // Verify style was removed
    auto get_style_result = run.get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_TRUE(get_style_result.value().empty());
}

// ============================================================================
// Table Style Application Tests
// ============================================================================

TEST_F(StyleApplicationTest, ApplyCustomTableStyle)
{
    // Create custom table style
    auto style_result = style_manager->create_table_style_safe("Custom Table");
    ASSERT_TRUE(style_result.ok());
    
    duckx::Style* style = style_result.value();
    duckx::TableStyleProperties props;
    props.border_style = "single";
    props.border_width_pts = 1.5;
    props.border_color_hex = "000000";
    auto props_result = style->set_table_properties_safe(props);
    ASSERT_TRUE(props_result.ok());

    // Add a table
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    duckx::Table* table = &table_result.value();

    // Apply table style
    auto apply_result = table->apply_style_safe(*style_manager, "Custom Table");
    EXPECT_TRUE(apply_result.ok());

    // Verify style was applied
    auto get_style_result = table->get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_EQ("Custom Table", get_style_result.value());
}

TEST_F(StyleApplicationTest, ApplyIncompatibleStyleToTable)
{
    // Create character-only style
    auto style_result = style_manager->create_character_style_safe("Char Only");
    ASSERT_TRUE(style_result.ok());

    // Add a table
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    duckx::Table* table = &table_result.value();

    // Try to apply character style to table (should fail)
    auto apply_result = table->apply_style_safe(*style_manager, "Char Only");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(duckx::ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(StyleApplicationTest, ApplyNonexistentStyle)
{
    // Add a paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    // Try to apply non-existent style
    auto apply_result = para->apply_style_safe(*style_manager, "Does Not Exist");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(duckx::ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

TEST_F(StyleApplicationTest, GetStyleFromElementWithoutStyle)
{
    // Add a paragraph without applying any style
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    // Get style (should return empty string)
    auto get_style_result = para->get_style_safe();
    EXPECT_TRUE(get_style_result.ok());
    EXPECT_TRUE(get_style_result.value().empty());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(StyleApplicationTest, CompleteStyleWorkflow)
{
    // Debug: Check initial style count
    std::cout << "Initial style count: " << style_manager->style_count() << std::endl;
    
    // Create custom styles
    auto para_style_result = style_manager->create_mixed_style_safe("Test Para");
    ASSERT_TRUE(para_style_result.ok()) << "Failed to create Test Para style: " << para_style_result.error().to_string();
    
    auto char_style_result = style_manager->create_character_style_safe("Test Char");
    ASSERT_TRUE(char_style_result.ok()) << "Failed to create Test Char style: " << char_style_result.error().to_string();
    
    // Debug: Check style count after creation
    std::cout << "Style count after creation: " << style_manager->style_count() << std::endl;
    
    // Debug: List all style names
    auto all_styles = style_manager->get_all_style_names();
    std::cout << "All styles: ";
    for (const auto& name : all_styles) {
        std::cout << "'" << name << "' ";
    }
    std::cout << std::endl;

    // Configure styles
    duckx::Style* para_style = para_style_result.value();
    duckx::ParagraphStyleProperties para_props;
    para_props.alignment = duckx::Alignment::CENTER;
    duckx::CharacterStyleProperties para_char_props;
    para_char_props.font_name = "Arial";
    para_char_props.font_size_pts = 16.0;
    auto para_props_result = para_style->set_paragraph_properties_safe(para_props);
    ASSERT_TRUE(para_props_result.ok()) << "Failed to set paragraph properties: " << para_props_result.error().to_string();
    auto char_props_result = para_style->set_character_properties_safe(para_char_props);
    ASSERT_TRUE(char_props_result.ok()) << "Failed to set character properties: " << char_props_result.error().to_string();

    duckx::Style* char_style = char_style_result.value();
    duckx::CharacterStyleProperties char_props;
    char_props.font_color_hex = "FF0000";
    char_props.formatting_flags = duckx::bold;
    char_style->set_character_properties_safe(char_props);

    // Create document content with applied styles
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    duckx::Paragraph* para = &para_result.value();

    // Verify style exists before applying
    auto style_check = style_manager->get_style_safe("Test Para");
    ASSERT_TRUE(style_check.ok()) << "Test Para style not found in style manager: " << style_check.error().to_string();
    
    // Apply paragraph style
    auto para_apply_result = para->apply_style_safe(*style_manager, "Test Para");
    EXPECT_TRUE(para_apply_result.ok()) << "Failed to apply Test Para style: " << para_apply_result.error().to_string();

    // Add runs with different character styles
    duckx::Run& normal_run = para->add_run("Normal text ");
    duckx::Run& styled_run = para->add_run("styled text");

    auto char_apply_result = styled_run.apply_style_safe(*style_manager, "Test Char");
    EXPECT_TRUE(char_apply_result.ok());

    // Verify all styles are applied correctly
    auto para_style_check = para->get_style_safe();
    EXPECT_TRUE(para_style_check.ok());
    EXPECT_EQ("Test Para", para_style_check.value());

    auto char_style_check = styled_run.get_style_safe();
    EXPECT_TRUE(char_style_check.ok());
    EXPECT_EQ("Test Char", char_style_check.value());

    auto normal_style_check = normal_run.get_style_safe();
    EXPECT_TRUE(normal_style_check.ok());
    EXPECT_TRUE(normal_style_check.value().empty()); // No style applied

    // Save document and verify it's created successfully
    auto save_result = doc->save_safe();
    EXPECT_TRUE(save_result.ok()) << "Failed to save document: " << save_result.error().to_string();
}

} // namespace test
} // namespace duckx