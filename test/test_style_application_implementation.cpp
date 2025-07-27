/*!
 * @file test_style_application_implementation.cpp
 * @brief Tests for the newly implemented style application methods
 * 
 * Tests the complete style application system including property application
 * and style compatibility validation.
 */

#include <gtest/gtest.h>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"

using namespace duckx;

class StyleApplicationImplementationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto doc_result = Document::create_safe("test_style_application_implementation.docx");
        ASSERT_TRUE(doc_result.ok()) << "Failed to create document: " << doc_result.error().to_string();
        doc = std::make_unique<Document>(std::move(doc_result.value()));
        
        body = &doc->body();
        
        style_manager = std::make_unique<StyleManager>();
        
        // Load built-in styles for testing
        auto load_result = style_manager->load_all_built_in_styles_safe();
        ASSERT_TRUE(load_result.ok()) << "Failed to load built-in styles: " << load_result.error().to_string();
    }

    std::unique_ptr<Document> doc;
    Body* body;
    std::unique_ptr<StyleManager> style_manager;
};

// ============================================================================
// Style Property Application Tests
// ============================================================================

TEST_F(StyleApplicationImplementationTest, ApplyParagraphPropertiesDirectly)
{
    // Create a paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Create paragraph properties
    ParagraphStyleProperties props;
    props.alignment = Alignment::CENTER;
    props.space_before_pts = 12.0;
    props.space_after_pts = 6.0;
    props.line_spacing = 1.5;
    
    // Apply properties directly using the new method
    auto apply_result = style_manager->apply_paragraph_properties_safe(*para, props);
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply paragraph properties: " << apply_result.error().to_string();
    
    // Verify properties were applied by reading them back
    auto read_result = style_manager->read_paragraph_properties_safe(*para);
    ASSERT_TRUE(read_result.ok());
    
    const auto& applied_props = read_result.value();
    EXPECT_TRUE(applied_props.alignment.has_value());
    EXPECT_EQ(Alignment::CENTER, applied_props.alignment.value());
    EXPECT_TRUE(applied_props.space_before_pts.has_value());
    EXPECT_DOUBLE_EQ(12.0, applied_props.space_before_pts.value());
    EXPECT_TRUE(applied_props.space_after_pts.has_value());
    EXPECT_DOUBLE_EQ(6.0, applied_props.space_after_pts.value());
}

TEST_F(StyleApplicationImplementationTest, ApplyCharacterPropertiesDirectly)
{
    // Create a run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    duckx::Run& run = para->add_run("Test text");
    
    // Create character properties
    CharacterStyleProperties props;
    props.font_name = "Arial";
    props.font_size_pts = 14.0;
    props.font_color_hex = "FF0000";
    props.formatting_flags = bold | italic;
    
    // Apply properties directly using the new method
    auto apply_result = style_manager->apply_character_properties_safe(run, props);
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply character properties: " << apply_result.error().to_string();
    
    // Verify properties were applied by reading them back
    auto read_result = style_manager->read_character_properties_safe(run);
    ASSERT_TRUE(read_result.ok());
    
    const auto& applied_props = read_result.value();
    EXPECT_TRUE(applied_props.font_name.has_value());
    EXPECT_EQ("Arial", applied_props.font_name.value());
    EXPECT_TRUE(applied_props.font_size_pts.has_value());
    EXPECT_DOUBLE_EQ(14.0, applied_props.font_size_pts.value());
    EXPECT_TRUE(applied_props.font_color_hex.has_value());
    EXPECT_EQ("FF0000", applied_props.font_color_hex.value());
}

TEST_F(StyleApplicationImplementationTest, ApplyTablePropertiesDirectly)
{
    // Create a table
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    // Create table properties
    TableStyleProperties props;
    props.table_width_pts = 500.0;
    props.table_alignment = "center";
    props.border_style = "single";
    props.border_width_pts = 1.0;
    props.border_color_hex = "000000";
    
    // Apply properties directly using the new method
    auto apply_result = style_manager->apply_table_properties_safe(*table, props);
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply table properties: " << apply_result.error().to_string();
    
    // Verify properties were applied by reading them back
    auto read_result = style_manager->read_table_properties_safe(*table);
    ASSERT_TRUE(read_result.ok());
    
    const auto& applied_props = read_result.value();
    EXPECT_TRUE(applied_props.table_width_pts.has_value());
    EXPECT_DOUBLE_EQ(500.0, applied_props.table_width_pts.value());
    EXPECT_TRUE(applied_props.table_alignment.has_value());
    EXPECT_EQ("center", applied_props.table_alignment.value());
    EXPECT_TRUE(applied_props.border_style.has_value());
    EXPECT_EQ("single", applied_props.border_style.value());
}

// ============================================================================
// Style Application via StyleManager Tests
// ============================================================================

TEST_F(StyleApplicationImplementationTest, ApplyParagraphStyleViaStyleManager)
{
    // Create a custom paragraph style
    auto style_result = style_manager->create_paragraph_style_safe("Test Para Style");
    ASSERT_TRUE(style_result.ok());
    Style* style = style_result.value();
    
    // Configure the style
    ParagraphStyleProperties props;
    props.alignment = Alignment::RIGHT;
    props.space_before_pts = 18.0;
    
    auto set_props_result = style->set_paragraph_properties_safe(props);
    ASSERT_TRUE(set_props_result.ok());
    
    // Create a paragraph and apply the style
    auto para_result = body->add_paragraph_safe("Styled paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_result = style_manager->apply_paragraph_style_safe(*para, "Test Para Style");
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply paragraph style: " << apply_result.error().to_string();
    
    // Verify the style was applied
    auto read_result = style_manager->read_paragraph_properties_safe(*para);
    ASSERT_TRUE(read_result.ok());
    
    const auto& applied_props = read_result.value();
    EXPECT_TRUE(applied_props.alignment.has_value());
    EXPECT_EQ(Alignment::RIGHT, applied_props.alignment.value());
    EXPECT_TRUE(applied_props.space_before_pts.has_value());
    EXPECT_DOUBLE_EQ(18.0, applied_props.space_before_pts.value());
}

TEST_F(StyleApplicationImplementationTest, ApplyCharacterStyleViaStyleManager)
{
    // Create a custom character style
    auto style_result = style_manager->create_character_style_safe("Test Char Style");
    ASSERT_TRUE(style_result.ok());
    Style* style = style_result.value();
    
    // Configure the style
    CharacterStyleProperties props;
    props.font_name = "Times New Roman";
    props.font_size_pts = 16.0;
    props.formatting_flags = bold;
    
    auto set_props_result = style->set_character_properties_safe(props);
    ASSERT_TRUE(set_props_result.ok());
    
    // Create a run and apply the style
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    duckx::Run& run = para->add_run("Styled text");
    
    auto apply_result = style_manager->apply_character_style_safe(run, "Test Char Style");
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply character style: " << apply_result.error().to_string();
    
    // Verify the style was applied
    auto read_result = style_manager->read_character_properties_safe(run);
    ASSERT_TRUE(read_result.ok());
    
    const auto& applied_props = read_result.value();
    EXPECT_TRUE(applied_props.font_name.has_value());
    EXPECT_EQ("Times New Roman", applied_props.font_name.value());
    EXPECT_TRUE(applied_props.font_size_pts.has_value());
    EXPECT_DOUBLE_EQ(16.0, applied_props.font_size_pts.value());
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(StyleApplicationImplementationTest, ApplyIncompatibleParagraphStyle)
{
    // Create a character-only style
    auto style_result = style_manager->create_character_style_safe("Char Only Style");
    ASSERT_TRUE(style_result.ok());
    
    // Try to apply it to a paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_result = style_manager->apply_paragraph_style_safe(*para, "Char Only Style");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

TEST_F(StyleApplicationImplementationTest, ApplyIncompatibleCharacterStyle)
{
    // Create a table-only style
    auto style_result = style_manager->create_table_style_safe("Table Only Style");
    ASSERT_TRUE(style_result.ok());
    
    // Try to apply it to a run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    duckx::Run& run = para->add_run("Test text");
    
    auto apply_result = style_manager->apply_character_style_safe(run, "Table Only Style");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

TEST_F(StyleApplicationImplementationTest, ApplyNonexistentStyle)
{
    // Try to apply a style that doesn't exist
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_result = style_manager->apply_paragraph_style_safe(*para, "Nonexistent Style");
    EXPECT_FALSE(apply_result.ok());
    EXPECT_EQ(ErrorCategory::STYLE_SYSTEM, apply_result.error().category());
}

// ============================================================================
// Mixed Style Tests
// ============================================================================

TEST_F(StyleApplicationImplementationTest, ApplyMixedStyleToParagraph)
{
    // Create a mixed style with both paragraph and character properties
    auto style_result = style_manager->create_mixed_style_safe("Mixed Style");
    ASSERT_TRUE(style_result.ok());
    Style* style = style_result.value();
    
    // Set paragraph properties
    ParagraphStyleProperties para_props;
    para_props.alignment = Alignment::CENTER;
    para_props.space_before_pts = 12.0;
    
    auto set_para_result = style->set_paragraph_properties_safe(para_props);
    ASSERT_TRUE(set_para_result.ok());
    
    // Apply to paragraph (should work since mixed styles support paragraphs)
    auto para_result = body->add_paragraph_safe("Mixed styled paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_result = style_manager->apply_paragraph_style_safe(*para, "Mixed Style");
    EXPECT_TRUE(apply_result.ok()) << "Failed to apply mixed style to paragraph: " << apply_result.error().to_string();
    
    // Verify paragraph properties were applied
    auto read_result = style_manager->read_paragraph_properties_safe(*para);
    ASSERT_TRUE(read_result.ok());
    
    const auto& applied_props = read_result.value();
    EXPECT_TRUE(applied_props.alignment.has_value());
    EXPECT_EQ(Alignment::CENTER, applied_props.alignment.value());
}