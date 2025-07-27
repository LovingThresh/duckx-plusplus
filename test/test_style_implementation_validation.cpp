/*!
 * @file test_style_implementation_validation.cpp
 * @brief Validation test to ensure all style application methods are implemented
 */

#include <gtest/gtest.h>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"

using namespace duckx;

class StyleImplementationValidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto doc_result = Document::create_safe("test_style_implementation_validation.docx");
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

TEST_F(StyleImplementationValidationTest, ValidateApplyParagraphPropertiesMethodExists)
{
    // Create test paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Create test properties
    ParagraphStyleProperties props;
    props.alignment = Alignment::CENTER;
    props.space_before_pts = 10.0;
    
    // This should compile and execute without error
    auto result = style_manager->apply_paragraph_properties_safe(*para, props);
    EXPECT_TRUE(result.ok()) << "apply_paragraph_properties_safe method should exist and work";
}

TEST_F(StyleImplementationValidationTest, ValidateApplyCharacterPropertiesMethodExists)
{
    // Create test run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    duckx::Run& run = para->add_run("Test text");
    
    // Create test properties
    CharacterStyleProperties props;
    props.font_name = "Arial";
    props.font_size_pts = 12.0;
    props.formatting_flags = bold;
    
    // This should compile and execute without error
    auto result = style_manager->apply_character_properties_safe(run, props);
    EXPECT_TRUE(result.ok()) << "apply_character_properties_safe method should exist and work";
}

TEST_F(StyleImplementationValidationTest, ValidateApplyTablePropertiesMethodExists)
{
    // Create test table
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    // Create test properties
    TableStyleProperties props;
    props.table_width_pts = 400.0;
    props.table_alignment = "center";
    
    // This should compile and execute without error
    auto result = style_manager->apply_table_properties_safe(*table, props);
    EXPECT_TRUE(result.ok()) << "apply_table_properties_safe method should exist and work";
}

TEST_F(StyleImplementationValidationTest, ValidateApplyParagraphStyleMethodExists)
{
    // Create custom style
    auto style_result = style_manager->create_paragraph_style_safe("Test Style");
    ASSERT_TRUE(style_result.ok());
    
    // Create test paragraph
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // This should compile and execute without error
    auto result = style_manager->apply_paragraph_style_safe(*para, "Test Style");
    EXPECT_TRUE(result.ok()) << "apply_paragraph_style_safe method should exist and work";
}

TEST_F(StyleImplementationValidationTest, ValidateApplyCharacterStyleMethodExists)
{
    // Create custom style
    auto style_result = style_manager->create_character_style_safe("Test Char Style");
    ASSERT_TRUE(style_result.ok());
    
    // Create test run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    duckx::Run& run = para->add_run("Test text");
    
    // This should compile and execute without error
    auto result = style_manager->apply_character_style_safe(run, "Test Char Style");
    EXPECT_TRUE(result.ok()) << "apply_character_style_safe method should exist and work";
}

TEST_F(StyleImplementationValidationTest, ValidateApplyTableStyleMethodExists)
{
    // Create custom style
    auto style_result = style_manager->create_table_style_safe("Test Table Style");
    ASSERT_TRUE(style_result.ok());
    
    // Create test table
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    // This should compile and execute without error
    auto result = style_manager->apply_table_style_safe(*table, "Test Table Style");
    EXPECT_TRUE(result.ok()) << "apply_table_style_safe method should exist and work";
}

TEST_F(StyleImplementationValidationTest, ValidateBuiltInStyleApplication)
{
    // Test applying a built-in style
    auto para_result = body->add_paragraph_safe("Test with built-in style");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Apply built-in Heading 1 style
    auto result = style_manager->apply_paragraph_style_safe(*para, "Heading 1");
    EXPECT_TRUE(result.ok()) << "Should be able to apply built-in Heading 1 style";
    
    // Verify we can read back the properties
    auto read_result = style_manager->read_paragraph_properties_safe(*para);
    EXPECT_TRUE(read_result.ok()) << "Should be able to read applied properties";
}