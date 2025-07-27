/*!
 * @file test_complete_style_implementation.cpp
 * @brief Final validation test for complete style implementation
 * 
 * This test validates that all required style methods are implemented
 * and work correctly together.
 */

#include <gtest/gtest.h>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"

using namespace duckx;

class CompleteStyleImplementationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto doc_result = Document::create_safe("test_complete_style_implementation.docx");
        ASSERT_TRUE(doc_result.ok());
        doc = std::make_unique<Document>(std::move(doc_result.value()));
        
        body = &doc->body();
        
        style_manager = std::make_unique<StyleManager>();
        
        auto load_result = style_manager->load_all_built_in_styles_safe();
        ASSERT_TRUE(load_result.ok());
    }

    std::unique_ptr<Document> doc;
    Body* body;
    std::unique_ptr<StyleManager> style_manager;
};

// ============================================================================
// Property Application Methods Test
// ============================================================================

TEST_F(CompleteStyleImplementationTest, AllPropertyApplicationMethodsWork)
{
    // Test paragraph properties application
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    ParagraphStyleProperties para_props;
    para_props.alignment = Alignment::CENTER;
    para_props.space_before_pts = 12.0;
    para_props.space_after_pts = 6.0;
    
    auto para_apply_result = style_manager->apply_paragraph_properties_safe(*para, para_props);
    EXPECT_TRUE(para_apply_result.ok()) << "apply_paragraph_properties_safe should work";
    
    // Test character properties application
    duckx::Run& run = para->add_run("Test text");
    
    CharacterStyleProperties char_props;
    char_props.font_name = "Arial";
    char_props.font_size_pts = 14.0;
    char_props.formatting_flags = bold | italic;
    
    auto char_apply_result = style_manager->apply_character_properties_safe(run, char_props);
    EXPECT_TRUE(char_apply_result.ok()) << "apply_character_properties_safe should work";
    
    // Test table properties application
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    TableStyleProperties table_props;
    table_props.table_width_pts = 500.0;
    table_props.table_alignment = "center";
    table_props.border_style = "single";
    
    auto table_apply_result = style_manager->apply_table_properties_safe(*table, table_props);
    EXPECT_TRUE(table_apply_result.ok()) << "apply_table_properties_safe should work";
}

// ============================================================================
// Style Application Methods Test
// ============================================================================

TEST_F(CompleteStyleImplementationTest, AllStyleApplicationMethodsWork)
{
    // Create custom styles
    auto para_style_result = style_manager->create_paragraph_style_safe("Custom Para");
    ASSERT_TRUE(para_style_result.ok());
    Style* para_style = para_style_result.value();
    
    ParagraphStyleProperties para_props;
    para_props.alignment = Alignment::RIGHT;
    para_props.space_before_pts = 10.0;
    auto set_para_result = para_style->set_paragraph_properties_safe(para_props);
    ASSERT_TRUE(set_para_result.ok());
    
    auto char_style_result = style_manager->create_character_style_safe("Custom Char");
    ASSERT_TRUE(char_style_result.ok());
    Style* char_style = char_style_result.value();
    
    CharacterStyleProperties char_props;
    char_props.font_name = "Times New Roman";
    char_props.font_size_pts = 16.0;
    char_props.formatting_flags = bold;
    auto set_char_result = char_style->set_character_properties_safe(char_props);
    ASSERT_TRUE(set_char_result.ok());
    
    auto table_style_result = style_manager->create_table_style_safe("Custom Table");
    ASSERT_TRUE(table_style_result.ok());
    Style* table_style = table_style_result.value();
    
    TableStyleProperties table_props;
    table_props.table_width_pts = 600.0;
    table_props.border_style = "double";
    auto set_table_result = table_style->set_table_properties_safe(table_props);
    ASSERT_TRUE(set_table_result.ok());
    
    // Test paragraph style application
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_para_result = style_manager->apply_paragraph_style_safe(*para, "Custom Para");
    EXPECT_TRUE(apply_para_result.ok()) << "apply_paragraph_style_safe should work";
    
    // Test character style application
    duckx::Run& run = para->add_run(" with styled text");
    
    auto apply_char_result = style_manager->apply_character_style_safe(run, "Custom Char");
    EXPECT_TRUE(apply_char_result.ok()) << "apply_character_style_safe should work";
    
    // Test table style application
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    auto apply_table_result = style_manager->apply_table_style_safe(*table, "Custom Table");
    EXPECT_TRUE(apply_table_result.ok()) << "apply_table_style_safe should work";
}

// ============================================================================
// Generic Style Application Test
// ============================================================================

TEST_F(CompleteStyleImplementationTest, GenericStyleApplicationWorks)
{
    // Create a mixed style
    auto mixed_style_result = style_manager->create_mixed_style_safe("Mixed Style");
    ASSERT_TRUE(mixed_style_result.ok());
    Style* mixed_style = mixed_style_result.value();
    
    // Set both paragraph and character properties
    ParagraphStyleProperties para_props;
    para_props.alignment = Alignment::CENTER;
    para_props.space_before_pts = 15.0;
    auto set_para_result = mixed_style->set_paragraph_properties_safe(para_props);
    ASSERT_TRUE(set_para_result.ok());
    
    CharacterStyleProperties char_props;
    char_props.font_name = "Calibri";
    char_props.font_size_pts = 18.0;
    auto set_char_result = mixed_style->set_character_properties_safe(char_props);
    ASSERT_TRUE(set_char_result.ok());
    
    // Test generic style application on paragraph
    auto para_result = body->add_paragraph_safe("Generic style test");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_result = style_manager->apply_style_safe(*para, "Mixed Style");
    EXPECT_TRUE(apply_result.ok()) << "Generic apply_style_safe should work on paragraph";
    
    // Test generic style application on run
    duckx::Run& run = para->add_run(" more text");
    
    auto apply_run_result = style_manager->apply_style_safe(run, "Mixed Style");
    EXPECT_TRUE(apply_run_result.ok()) << "Generic apply_style_safe should work on run";
}

// ============================================================================
// Complete Workflow Test
// ============================================================================

TEST_F(CompleteStyleImplementationTest, CompleteStyleWorkflowWorks)
{
    // 1. Create and configure a style
    auto style_result = style_manager->create_mixed_style_safe("Workflow Test Style");
    ASSERT_TRUE(style_result.ok());
    Style* style = style_result.value();
    
    ParagraphStyleProperties para_props;
    para_props.alignment = Alignment::CENTER;
    para_props.space_before_pts = 20.0;
    para_props.space_after_pts = 10.0;
    auto set_para_result = style->set_paragraph_properties_safe(para_props);
    ASSERT_TRUE(set_para_result.ok());
    
    CharacterStyleProperties char_props;
    char_props.font_name = "Georgia";
    char_props.font_size_pts = 20.0;
    char_props.font_color_hex = "800080";
    char_props.formatting_flags = bold | italic;
    auto set_char_result = style->set_character_properties_safe(char_props);
    ASSERT_TRUE(set_char_result.ok());
    
    // 2. Apply the style to elements
    auto para_result = body->add_paragraph_safe("Complete workflow test");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    auto apply_para_result = style_manager->apply_paragraph_style_safe(*para, "Workflow Test Style");
    ASSERT_TRUE(apply_para_result.ok());
    
    duckx::Run& run = para->add_run(" - with character formatting");
    auto apply_char_result = style_manager->apply_character_style_safe(run, "Workflow Test Style");
    ASSERT_TRUE(apply_char_result.ok());
    
    // 3. Read back the applied properties
    auto read_para_result = style_manager->read_paragraph_properties_safe(*para);
    ASSERT_TRUE(read_para_result.ok());
    const auto& applied_para_props = read_para_result.value();
    
    EXPECT_TRUE(applied_para_props.alignment.has_value());
    EXPECT_EQ(Alignment::CENTER, applied_para_props.alignment.value());
    EXPECT_TRUE(applied_para_props.space_before_pts.has_value());
    EXPECT_DOUBLE_EQ(20.0, applied_para_props.space_before_pts.value());
    
    auto read_char_result = style_manager->read_character_properties_safe(run);
    ASSERT_TRUE(read_char_result.ok());
    const auto& applied_char_props = read_char_result.value();
    
    EXPECT_TRUE(applied_char_props.font_name.has_value());
    EXPECT_EQ("Georgia", applied_char_props.font_name.value());
    EXPECT_TRUE(applied_char_props.font_size_pts.has_value());
    EXPECT_DOUBLE_EQ(20.0, applied_char_props.font_size_pts.value());
    EXPECT_TRUE(applied_char_props.font_color_hex.has_value());
    EXPECT_EQ("800080", applied_char_props.font_color_hex.value());
    
    // 4. Test style extraction from formatted element
    auto extract_result = style_manager->extract_style_from_element_safe(*para, "Extracted Style");
    EXPECT_TRUE(extract_result.ok()) << "Should be able to extract style from formatted element";
    
    if (extract_result.ok()) {
        Style* extracted_style = extract_result.value();
        EXPECT_EQ("Extracted Style", extracted_style->name());
        EXPECT_EQ(StyleType::MIXED, extracted_style->type());
        
        // Verify extracted properties match
        const auto& extracted_para_props = extracted_style->paragraph_properties();
        EXPECT_TRUE(extracted_para_props.alignment.has_value());
        EXPECT_EQ(Alignment::CENTER, extracted_para_props.alignment.value());
    }
    
    std::cout << "\n✅ All style implementation methods are working correctly!" << std::endl;
    std::cout << "   Total styles in manager: " << style_manager->style_count() << std::endl;
}