/*!
 * @file test_style_reading.cpp  
 * @brief Tests for style reading and extraction functionality
 * 
 * Tests comprehensive style reading capabilities including property extraction,
 * inheritance resolution, and style comparison features.
 */

#include <gtest/gtest.h>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"

using namespace duckx;

class StyleReadingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto doc_result = Document::create_safe("test_style_reading.docx");
        ASSERT_TRUE(doc_result.ok()) << "Failed to create document: " << doc_result.error().to_string();
        doc = std::make_unique<Document>(std::move(doc_result.value()));
        
        body = &doc->body();
        
        style_manager = std::make_unique<StyleManager>();
        
        // Load some built-in styles for testing
        auto load_result = style_manager->load_all_built_in_styles_safe();
        ASSERT_TRUE(load_result.ok()) << "Failed to load built-in styles: " << load_result.error().to_string();
    }

    std::unique_ptr<Document> doc;
    Body* body;
    std::unique_ptr<StyleManager> style_manager;
};

// ============================================================================
// Paragraph Style Reading Tests
// ============================================================================

TEST_F(StyleReadingTest, ReadEmptyParagraphProperties)
{
    // Add a plain paragraph with no formatting
    auto para_result = body->add_paragraph_safe("Plain text paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Read its properties
    auto props_result = style_manager->read_paragraph_properties_safe(*para);
    EXPECT_TRUE(props_result.ok());
    
    const auto& props = props_result.value();
    
    // Should have no specific properties set
    EXPECT_FALSE(props.alignment.has_value());
    EXPECT_FALSE(props.space_before_pts.has_value());
    EXPECT_FALSE(props.space_after_pts.has_value());
    EXPECT_FALSE(props.line_spacing.has_value());
    EXPECT_FALSE(props.left_indent_pts.has_value());
    EXPECT_FALSE(props.right_indent_pts.has_value());
    EXPECT_FALSE(props.first_line_indent_pts.has_value());
}

TEST_F(StyleReadingTest, ReadParagraphWithAppliedStyle)
{
    // Add a paragraph and apply a style
    auto para_result = body->add_paragraph_safe("Styled paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Apply Heading 1 style
    auto apply_result = para->apply_style_safe(*style_manager, "Heading 1");
    ASSERT_TRUE(apply_result.ok());
    
    // Read effective properties (should include inheritance)
    auto effective_result = style_manager->get_effective_paragraph_properties_safe(*para);
    EXPECT_TRUE(effective_result.ok());
    
    const auto& effective_props = effective_result.value();
    
    // Heading 1 typically has specific font size and spacing
    // The exact values depend on the built-in style definition
    EXPECT_TRUE(effective_props.space_after_pts.has_value() || 
                effective_props.space_before_pts.has_value());
}

TEST_F(StyleReadingTest, ReadParagraphWithDirectFormatting)
{
    // Add a paragraph and apply direct formatting
    auto para_result = body->add_paragraph_safe("Formatted paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Apply some direct formatting using existing methods
    para->set_alignment(Alignment::CENTER);
    para->set_spacing(12.0, -1);  // Set before spacing to 12.0, leave after as default
    
    // Read properties
    auto props_result = style_manager->read_paragraph_properties_safe(*para);
    EXPECT_TRUE(props_result.ok());
    
    const auto& props = props_result.value();
    
    // Should reflect the direct formatting
    EXPECT_TRUE(props.alignment.has_value());
    EXPECT_EQ(Alignment::CENTER, props.alignment.value());
    EXPECT_TRUE(props.space_before_pts.has_value());
    EXPECT_DOUBLE_EQ(12.0, props.space_before_pts.value());
}

// ============================================================================
// Character Style Reading Tests
// ============================================================================

TEST_F(StyleReadingTest, ReadEmptyCharacterProperties)
{
    // Add a paragraph with a plain run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    duckx::Run& run = para->add_run("Plain text");
    
    // Read its properties
    auto props_result = style_manager->read_character_properties_safe(run);
    EXPECT_TRUE(props_result.ok());
    
    const auto& props = props_result.value();
    
    // Should have no specific properties set
    EXPECT_FALSE(props.font_name.has_value());
    EXPECT_FALSE(props.font_size_pts.has_value());
    EXPECT_FALSE(props.font_color_hex.has_value());
    EXPECT_FALSE(props.highlight_color.has_value());
    EXPECT_FALSE(props.formatting_flags.has_value());
}

TEST_F(StyleReadingTest, ReadCharacterWithAppliedStyle)
{
    // Add a paragraph with a run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    duckx::Run& run = para->add_run("Styled text");
    
    // Apply a character style
    auto apply_result = run.apply_style_safe(*style_manager, "Code");
    ASSERT_TRUE(apply_result.ok());
    
    // Read effective properties
    auto effective_result = style_manager->get_effective_character_properties_safe(run);
    EXPECT_TRUE(effective_result.ok());
    
    const auto& effective_props = effective_result.value();
    
    // Code style typically has monospace font
    EXPECT_TRUE(effective_props.font_name.has_value());
    EXPECT_TRUE(effective_props.font_size_pts.has_value());
}

TEST_F(StyleReadingTest, ReadCharacterWithDirectFormatting)
{
    // Add a paragraph with a run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Create run with formatting flags
    duckx::Run& run = para->add_run("Formatted text", bold | italic);
    
    // Apply some direct formatting using existing methods
    run.set_font("Arial").set_font_size(14.0);
    
    // Read properties
    auto props_result = style_manager->read_character_properties_safe(run);
    EXPECT_TRUE(props_result.ok());
    
    const auto& props = props_result.value();
    
    // Should reflect the direct formatting
    EXPECT_TRUE(props.font_name.has_value());
    EXPECT_EQ("Arial", props.font_name.value());
    EXPECT_TRUE(props.font_size_pts.has_value());
    EXPECT_DOUBLE_EQ(14.0, props.font_size_pts.value());
    
    // Check formatting flags if they were captured
    if (props.formatting_flags.has_value()) {
        EXPECT_TRUE(props.formatting_flags.value() & bold);
        EXPECT_TRUE(props.formatting_flags.value() & italic);
    }
}

// ============================================================================
// Table Style Reading Tests
// ============================================================================

TEST_F(StyleReadingTest, ReadEmptyTableProperties)
{
    // Add a plain table
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    // Read its properties
    auto props_result = style_manager->read_table_properties_safe(*table);
    EXPECT_TRUE(props_result.ok());
    
    const auto& props = props_result.value();
    
    // Should have default border properties (tables are created with default borders)
    EXPECT_TRUE(props.border_style.has_value());
    EXPECT_EQ("single", props.border_style.value());
    // Other properties should not be set
    EXPECT_FALSE(props.cell_padding_pts.has_value());
    EXPECT_FALSE(props.table_width_pts.has_value());
    EXPECT_FALSE(props.table_alignment.has_value());
}

TEST_F(StyleReadingTest, ReadTableWithDirectFormatting)
{
    // Add a table and apply direct formatting
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    // Apply some formatting using existing methods
    table->set_width(400.0);
    table->set_alignment("center");
    table->set_border_style("single");
    
    // Read properties
    auto props_result = style_manager->read_table_properties_safe(*table);
    EXPECT_TRUE(props_result.ok());
    
    const auto& props = props_result.value();
    
    // Should reflect the direct formatting
    EXPECT_TRUE(props.table_width_pts.has_value());
    EXPECT_DOUBLE_EQ(400.0, props.table_width_pts.value());
    EXPECT_TRUE(props.table_alignment.has_value());
    EXPECT_EQ("center", props.table_alignment.value());
    EXPECT_TRUE(props.border_style.has_value());
    EXPECT_EQ("single", props.border_style.value());
}

// ============================================================================
// Style Extraction Tests
// ============================================================================

TEST_F(StyleReadingTest, ExtractStyleFromParagraph)
{
    // Create a paragraph with formatting
    auto para_result = body->add_paragraph_safe("Sample paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Apply formatting using existing methods
    para->set_alignment(Alignment::CENTER);
    para->set_spacing(6.0, -1);  // Set before spacing to 6.0
    
    // Extract style from the formatted paragraph
    auto extract_result = style_manager->extract_style_from_element_safe(*para, "Extracted Style");
    EXPECT_TRUE(extract_result.ok());
    
    Style* extracted_style = extract_result.value();
    EXPECT_EQ("Extracted Style", extracted_style->name());
    EXPECT_EQ(StyleType::MIXED, extracted_style->type());
    
    // Check that the properties were captured
    const auto& para_props = extracted_style->paragraph_properties();
    EXPECT_TRUE(para_props.alignment.has_value());
    EXPECT_EQ(Alignment::CENTER, para_props.alignment.value());
    EXPECT_TRUE(para_props.space_before_pts.has_value());
    EXPECT_DOUBLE_EQ(6.0, para_props.space_before_pts.value());
}

TEST_F(StyleReadingTest, ExtractStyleFromRun)
{
    // Create a run with formatting
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Create run with bold formatting
    duckx::Run& run = para->add_run("Sample run", bold);
    
    // Apply formatting using existing methods
    run.set_font("Times New Roman").set_font_size(12.0);
    
    // Extract style from the formatted run
    auto extract_result = style_manager->extract_style_from_element_safe(run, "Extracted Char Style");
    EXPECT_TRUE(extract_result.ok());
    
    Style* extracted_style = extract_result.value();
    EXPECT_EQ("Extracted Char Style", extracted_style->name());
    EXPECT_EQ(StyleType::CHARACTER, extracted_style->type());
    
    // Check that the properties were captured
    const auto& char_props = extracted_style->character_properties();
    EXPECT_TRUE(char_props.font_name.has_value());
    EXPECT_EQ("Times New Roman", char_props.font_name.value());
    EXPECT_TRUE(char_props.font_size_pts.has_value());
    EXPECT_DOUBLE_EQ(12.0, char_props.font_size_pts.value());
    
    // Check formatting flags if they were captured
    if (char_props.formatting_flags.has_value()) {
        EXPECT_TRUE(char_props.formatting_flags.value() & bold);
    }
}

// ============================================================================
// Style Comparison Tests
// ============================================================================

TEST_F(StyleReadingTest, CompareIdenticalStyles)
{
    // Create two identical custom styles
    auto style1_result = style_manager->create_paragraph_style_safe("Test Style 1");
    ASSERT_TRUE(style1_result.ok());
    Style* style1 = style1_result.value();
    
    auto style2_result = style_manager->create_paragraph_style_safe("Test Style 2");
    ASSERT_TRUE(style2_result.ok());
    Style* style2 = style2_result.value();
    
    // Set identical properties
    ParagraphStyleProperties props;
    props.alignment = Alignment::CENTER;
    props.space_before_pts = 6.0;
    
    auto set1_result = style1->set_paragraph_properties_safe(props);
    ASSERT_TRUE(set1_result.ok());
    
    auto set2_result = style2->set_paragraph_properties_safe(props);
    ASSERT_TRUE(set2_result.ok());
    
    // Compare the styles
    auto compare_result = style_manager->compare_styles_safe("Test Style 1", "Test Style 2");
    EXPECT_TRUE(compare_result.ok());
    
    std::string report = compare_result.value();
    EXPECT_TRUE(report.find("identical") != std::string::npos);
}

TEST_F(StyleReadingTest, CompareDifferentStyles)
{
    // Create two different styles
    auto style1_result = style_manager->create_paragraph_style_safe("Different Style 1");
    ASSERT_TRUE(style1_result.ok());
    Style* style1 = style1_result.value();
    
    auto style2_result = style_manager->create_paragraph_style_safe("Different Style 2");
    ASSERT_TRUE(style2_result.ok());
    Style* style2 = style2_result.value();
    
    // Set different properties
    ParagraphStyleProperties props1;
    props1.alignment = Alignment::LEFT;
    props1.space_before_pts = 6.0;
    
    ParagraphStyleProperties props2;
    props2.alignment = Alignment::CENTER;
    props2.space_before_pts = 12.0;
    
    auto set1_result = style1->set_paragraph_properties_safe(props1);
    ASSERT_TRUE(set1_result.ok());
    
    auto set2_result = style2->set_paragraph_properties_safe(props2);
    ASSERT_TRUE(set2_result.ok());
    
    // Compare the styles
    auto compare_result = style_manager->compare_styles_safe("Different Style 1", "Different Style 2");
    EXPECT_TRUE(compare_result.ok());
    
    std::string report = compare_result.value();
    EXPECT_TRUE(report.find("Alignment differs") != std::string::npos);
    EXPECT_TRUE(report.find("Space before differs") != std::string::npos);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(StyleReadingTest, ExtractStyleWithEmptyName)
{
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Try to extract with empty name
    auto extract_result = style_manager->extract_style_from_element_safe(*para, "");
    EXPECT_FALSE(extract_result.ok());
    EXPECT_EQ(ErrorCategory::VALIDATION, extract_result.error().category());
}

TEST_F(StyleReadingTest, CompareNonexistentStyles)
{
    // Try to compare styles that don't exist
    auto compare_result = style_manager->compare_styles_safe("NonExistent1", "NonExistent2");
    EXPECT_FALSE(compare_result.ok());
    EXPECT_EQ(ErrorCategory::STYLE_SYSTEM, compare_result.error().category());
}

// ============================================================================
// Inheritance Resolution Tests
// ============================================================================

TEST_F(StyleReadingTest, ResolveStyleInheritance)
{
    // Create a base style
    auto base_result = style_manager->create_paragraph_style_safe("Base Style");
    ASSERT_TRUE(base_result.ok());
    Style* base_style = base_result.value();
    
    ParagraphStyleProperties base_props;
    base_props.alignment = Alignment::LEFT;
    base_props.space_before_pts = 6.0;
    base_props.space_after_pts = 6.0;
    
    auto set_base_result = base_style->set_paragraph_properties_safe(base_props);
    ASSERT_TRUE(set_base_result.ok());
    
    // Create a derived style
    auto derived_result = style_manager->create_paragraph_style_safe("Derived Style");
    ASSERT_TRUE(derived_result.ok());
    Style* derived_style = derived_result.value();
    
    // Set inheritance
    auto inherit_result = derived_style->set_base_style_safe("Base Style");
    ASSERT_TRUE(inherit_result.ok());
    
    // Override some properties
    ParagraphStyleProperties derived_props;
    derived_props.alignment = Alignment::CENTER; // Override alignment
    // Leave spacing from base style
    
    auto set_derived_result = derived_style->set_paragraph_properties_safe(derived_props);
    ASSERT_TRUE(set_derived_result.ok());
    
    // Test inheritance resolution by applying the derived style to a paragraph
    // and reading back the effective properties
    auto test_para_result = body->add_paragraph_safe("Test inheritance");
    ASSERT_TRUE(test_para_result.ok());
    Paragraph* test_para = &test_para_result.value();
    
    auto apply_derived_result = style_manager->apply_paragraph_style_safe(*test_para, "Derived Style");
    EXPECT_TRUE(apply_derived_result.ok());
    
    if (apply_derived_result.ok()) {
        auto effective_result = style_manager->get_effective_paragraph_properties_safe(*test_para);
        EXPECT_TRUE(effective_result.ok());
        
        if (effective_result.ok()) {
            const auto& resolved = effective_result.value();
            
            // Should have the overridden alignment from derived style
            EXPECT_TRUE(resolved.alignment.has_value());
            EXPECT_EQ(Alignment::CENTER, resolved.alignment.value());
            
            // Should inherit spacing from base style
            EXPECT_TRUE(resolved.space_before_pts.has_value());
            EXPECT_DOUBLE_EQ(6.0, resolved.space_before_pts.value());
            EXPECT_TRUE(resolved.space_after_pts.has_value());
            EXPECT_DOUBLE_EQ(6.0, resolved.space_after_pts.value());
        }
    }
}