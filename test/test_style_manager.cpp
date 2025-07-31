/*!
 * @file test_style_manager.cpp
 * @brief Unit tests for StyleManager class and style operations
 * 
 * Tests comprehensive style management system including style creation,
 * inheritance, built-in libraries, property application, and XML generation.
 * Validates Result<T> error handling and style validation mechanisms.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include "StyleManager.hpp"

using namespace duckx;

class StyleManagerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            style_manager = std::make_unique<StyleManager>();
        }

        void TearDown() override
        {
            style_manager.reset();
        }

        std::unique_ptr<StyleManager> style_manager;
    };

    // ============================================================================
    // Basic Style Creation Tests
    // ============================================================================

    TEST_F(StyleManagerTest, CreateParagraphStyle)
    {
        auto result = style_manager->create_paragraph_style_safe("TestParagraph");
        ASSERT_TRUE(result.ok()) << "Failed to create paragraph style: " << result.error().to_string();

        Style* style = result.value();
        EXPECT_EQ(style->name(), "TestParagraph");
        EXPECT_EQ(style->type(), StyleType::PARAGRAPH);
        EXPECT_FALSE(style->is_built_in());
        EXPECT_TRUE(style->is_custom());
    }

    TEST_F(StyleManagerTest, CreateCharacterStyle)
    {
        auto result = style_manager->create_character_style_safe("TestCharacter");
        ASSERT_TRUE(result.ok()) << "Failed to create character style: " << result.error().to_string();

        Style* style = result.value();
        EXPECT_EQ(style->name(), "TestCharacter");
        EXPECT_EQ(style->type(), StyleType::CHARACTER);
        EXPECT_FALSE(style->is_built_in());
    }

    TEST_F(StyleManagerTest, CreateTableStyle)
    {
        auto result = style_manager->create_table_style_safe("TestTable");
        ASSERT_TRUE(result.ok()) << "Failed to create table style: " << result.error().to_string();

        Style* style = result.value();
        EXPECT_EQ(style->name(), "TestTable");
        EXPECT_EQ(style->type(), StyleType::TABLE);
        EXPECT_FALSE(style->is_built_in());
    }

    TEST_F(StyleManagerTest, CreateMixedStyle)
    {
        auto result = style_manager->create_mixed_style_safe("TestMixed");
        ASSERT_TRUE(result.ok()) << "Failed to create mixed style: " << result.error().to_string();

        Style* style = result.value();
        EXPECT_EQ(style->name(), "TestMixed");
        EXPECT_EQ(style->type(), StyleType::MIXED);
        EXPECT_FALSE(style->is_built_in());

        // Test that mixed style can accept both paragraph and character properties
        ParagraphStyleProperties para_props;
        para_props.alignment = Alignment::CENTER;
        auto para_result = style->set_paragraph_properties_safe(para_props);
        EXPECT_TRUE(para_result.ok());

        CharacterStyleProperties char_props;
        char_props.font_name = "Arial";
        char_props.font_size_pts = 12.0;
        auto char_result = style->set_character_properties_safe(char_props);
        EXPECT_TRUE(char_result.ok());
    }

    TEST_F(StyleManagerTest, CreateDuplicateStyleFails)
    {
        // Create first style
        auto result1 = style_manager->create_paragraph_style_safe("Duplicate");
        ASSERT_TRUE(result1.ok());

        // Attempt to create duplicate should fail
        auto result2 = style_manager->create_paragraph_style_safe("Duplicate");
        EXPECT_FALSE(result2.ok());
        EXPECT_EQ(result2.error().code(), ErrorCode::STYLE_ALREADY_EXISTS);
    }

    TEST_F(StyleManagerTest, CreateStyleWithEmptyNameFails)
    {
        auto result = style_manager->create_paragraph_style_safe("");
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.error().code(), ErrorCode::INVALID_ARGUMENT);
    }

    // ============================================================================
    // Style Retrieval Tests
    // ============================================================================

    TEST_F(StyleManagerTest, GetExistingStyle)
    {
        // Create a style
        auto create_result = style_manager->create_paragraph_style_safe("GetTest");
        ASSERT_TRUE(create_result.ok());

        // Retrieve it
        auto get_result = style_manager->get_style_safe("GetTest");
        ASSERT_TRUE(get_result.ok());

        Style* style = get_result.value();
        EXPECT_EQ(style->name(), "GetTest");
        EXPECT_EQ(style->type(), StyleType::PARAGRAPH);
    }

    TEST_F(StyleManagerTest, GetNonExistentStyleFails)
    {
        auto result = style_manager->get_style_safe("NonExistent");
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.error().code(), ErrorCode::STYLE_NOT_FOUND);
    }

    TEST_F(StyleManagerTest, HasStyleCheck)
    {
        EXPECT_FALSE(style_manager->has_style("TestStyle"));

        auto create_result = style_manager->create_paragraph_style_safe("TestStyle");
        ASSERT_TRUE(create_result.ok());

        EXPECT_TRUE(style_manager->has_style("TestStyle"));
    }

    // ============================================================================
    // Style Property Tests
    // ============================================================================

    TEST_F(StyleManagerTest, SetParagraphProperties)
    {
        auto style_result = style_manager->create_paragraph_style_safe("ParaProps");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        ParagraphStyleProperties props;
        props.alignment = Alignment::CENTER;
        props.space_before_pts = 12.0;
        props.space_after_pts = 6.0;
        props.line_spacing = 1.5;

        auto set_result = style->set_paragraph_properties_safe(props);
        EXPECT_TRUE(set_result.ok()) << "Failed to set paragraph properties: " << set_result.error().to_string();

        const auto& retrieved_props = style->paragraph_properties();
        EXPECT_EQ(retrieved_props.alignment.value(), Alignment::CENTER);
        EXPECT_EQ(retrieved_props.space_before_pts.value(), 12.0);
        EXPECT_EQ(retrieved_props.space_after_pts.value(), 6.0);
        EXPECT_EQ(retrieved_props.line_spacing.value(), 1.5);
    }

    TEST_F(StyleManagerTest, SetCharacterProperties)
    {
        auto style_result = style_manager->create_character_style_safe("CharProps");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        CharacterStyleProperties props;
        props.font_name = "Arial";
        props.font_size_pts = 14.0;
        props.font_color_hex = "FF0000";
        props.formatting_flags = bold | italic;

        auto set_result = style->set_character_properties_safe(props);
        EXPECT_TRUE(set_result.ok());

        const auto& retrieved_props = style->character_properties();
        EXPECT_EQ(retrieved_props.font_name.value(), "Arial");
        EXPECT_EQ(retrieved_props.font_size_pts.value(), 14.0);
        EXPECT_EQ(retrieved_props.font_color_hex.value(), "FF0000");
        EXPECT_EQ(retrieved_props.formatting_flags.value(), bold | italic);
    }

    TEST_F(StyleManagerTest, SetInvalidPropertiesOnWrongStyleType)
    {
        auto style_result = style_manager->create_character_style_safe("CharOnly");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        // Try to set paragraph properties on character style - should fail
        ParagraphStyleProperties para_props;
        para_props.alignment = Alignment::CENTER;

        auto result = style->set_paragraph_properties_safe(para_props);
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.error().code(), ErrorCode::STYLE_PROPERTY_INVALID);
    }

    // ============================================================================
    // Style Convenience Method Tests
    // ============================================================================

    TEST_F(StyleManagerTest, SetFontConvenienceMethod)
    {
        auto style_result = style_manager->create_character_style_safe("FontTest");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        auto font_result = style->set_font_safe("Times New Roman", 12.0);
        EXPECT_TRUE(font_result.ok());

        const auto& props = style->character_properties();
        EXPECT_EQ(props.font_name.value(), "Times New Roman");
        EXPECT_EQ(props.font_size_pts.value(), 12.0);
    }

    TEST_F(StyleManagerTest, SetInvalidFontSize)
    {
        auto style_result = style_manager->create_character_style_safe("FontTest");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        // Test negative font size
        auto result1 = style->set_font_safe("Arial", -5.0);
        EXPECT_FALSE(result1.ok());
        EXPECT_EQ(result1.error().code(), ErrorCode::INVALID_FONT_SIZE);

        // Test excessive font size
        auto result2 = style->set_font_safe("Arial", 2000.0);
        EXPECT_FALSE(result2.ok());
        EXPECT_EQ(result2.error().code(), ErrorCode::INVALID_FONT_SIZE);
    }

    TEST_F(StyleManagerTest, SetColorConvenienceMethod)
    {
        auto style_result = style_manager->create_character_style_safe("ColorTest");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        // Test valid hex colors
        auto result1 = style->set_color_safe("#FF0000");
        EXPECT_TRUE(result1.ok());

        auto result2 = style->set_color_safe("00FF00");
        EXPECT_TRUE(result2.ok());

        const auto& props = style->character_properties();
        EXPECT_EQ(props.font_color_hex.value(), "00FF00");
    }

    TEST_F(StyleManagerTest, SetInvalidColor)
    {
        auto style_result = style_manager->create_character_style_safe("ColorTest");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        // Test invalid hex color
        auto result1 = style->set_color_safe("INVALID");
        EXPECT_FALSE(result1.ok());
        EXPECT_EQ(result1.error().code(), ErrorCode::INVALID_COLOR_FORMAT);

        // Test too short color
        auto result2 = style->set_color_safe("FF");
        EXPECT_FALSE(result2.ok());
        EXPECT_EQ(result2.error().code(), ErrorCode::INVALID_COLOR_FORMAT);
    }

    // ============================================================================
    // Built-in Style Tests
    // ============================================================================

    TEST_F(StyleManagerTest, LoadBuiltInHeadingStyles)
    {
        auto result = style_manager->load_built_in_styles_safe(BuiltInStyleCategory::HEADING);
        EXPECT_TRUE(result.ok()) << "Failed to load built-in heading styles: " << result.error().to_string();

        // Check that heading styles are available
        for (int i = 1; i <= 6; ++i) {
            std::string heading_name = absl::StrFormat("Heading %d", i);
            EXPECT_TRUE(style_manager->has_style(heading_name)) 
                << "Missing built-in style: " << heading_name;

            auto style_result = style_manager->get_style_safe(heading_name);
            ASSERT_TRUE(style_result.ok());
            Style* style = style_result.value();
            EXPECT_TRUE(style->is_built_in());
            EXPECT_EQ(style->type(), StyleType::MIXED);
        }
    }

    TEST_F(StyleManagerTest, LoadBuiltInBodyTextStyles)
    {
        auto result = style_manager->load_built_in_styles_safe(BuiltInStyleCategory::BODY_TEXT);
        EXPECT_TRUE(result.ok());

        EXPECT_TRUE(style_manager->has_style("Normal"));
        auto normal_result = style_manager->get_style_safe("Normal");
        ASSERT_TRUE(normal_result.ok());
        EXPECT_TRUE(normal_result.value()->is_built_in());
    }

    TEST_F(StyleManagerTest, LoadAllBuiltInStyles)
    {
        auto result = style_manager->load_all_built_in_styles_safe();
        EXPECT_TRUE(result.ok()) << "Failed to load all built-in styles: " << result.error().to_string();

        // Check that we have styles from multiple categories
        EXPECT_TRUE(style_manager->has_style("Heading 1"));
        EXPECT_TRUE(style_manager->has_style("Normal"));
        EXPECT_TRUE(style_manager->has_style("Code"));

        // Check style count matches actual implementation:
        // Heading: 6 styles (Heading 1-6)
        // Body Text: 1 style (Normal)  
        // List: 0 styles (placeholder)
        // Table: 0 styles (placeholder)
        // Technical: 1 style (Code)
        // Total: 8 styles
        EXPECT_EQ(style_manager->style_count(), 8);
    }

    // ============================================================================
    // Style Management Tests
    // ============================================================================

    TEST_F(StyleManagerTest, GetAllStyleNames)
    {
        // Create some test styles
        style_manager->create_paragraph_style_safe("Test1");
        style_manager->create_character_style_safe("Test2");
        style_manager->create_table_style_safe("Test3");

        auto names = style_manager->get_all_style_names();
        EXPECT_EQ(names.size(), 3);
        
        std::sort(names.begin(), names.end());
        EXPECT_EQ(names[0], "Test1");
        EXPECT_EQ(names[1], "Test2");
        EXPECT_EQ(names[2], "Test3");
    }

    TEST_F(StyleManagerTest, GetStyleNamesByType)
    {
        style_manager->create_paragraph_style_safe("Para1");
        style_manager->create_paragraph_style_safe("Para2");
        style_manager->create_character_style_safe("Char1");

        auto para_names = style_manager->get_style_names_by_type(StyleType::PARAGRAPH);
        EXPECT_EQ(para_names.size(), 2);

        auto char_names = style_manager->get_style_names_by_type(StyleType::CHARACTER);
        EXPECT_EQ(char_names.size(), 1);
        EXPECT_EQ(char_names[0], "Char1");
    }

    TEST_F(StyleManagerTest, RemoveStyle)
    {
        auto create_result = style_manager->create_paragraph_style_safe("ToRemove");
        ASSERT_TRUE(create_result.ok());

        EXPECT_TRUE(style_manager->has_style("ToRemove"));

        auto remove_result = style_manager->remove_style_safe("ToRemove");
        EXPECT_TRUE(remove_result.ok());

        EXPECT_FALSE(style_manager->has_style("ToRemove"));
    }

    TEST_F(StyleManagerTest, RemoveNonExistentStyleFails)
    {
        auto result = style_manager->remove_style_safe("DoesNotExist");
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.error().code(), ErrorCode::STYLE_NOT_FOUND);
    }

    // ============================================================================
    // Style Inheritance Tests
    // ============================================================================

    TEST_F(StyleManagerTest, SetBaseStyle)
    {
        auto base_result = style_manager->create_paragraph_style_safe("BaseStyle");
        ASSERT_TRUE(base_result.ok());

        auto derived_result = style_manager->create_paragraph_style_safe("DerivedStyle");
        ASSERT_TRUE(derived_result.ok());
        Style* derived = derived_result.value();

        auto inherit_result = derived->set_base_style_safe("BaseStyle");
        EXPECT_TRUE(inherit_result.ok());

        EXPECT_TRUE(derived->base_style().has_value());
        EXPECT_EQ(derived->base_style().value(), "BaseStyle");
    }

    TEST_F(StyleManagerTest, PreventSelfInheritance)
    {
        auto style_result = style_manager->create_paragraph_style_safe("SelfRef");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        auto result = style->set_base_style_safe("SelfRef");
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.error().code(), ErrorCode::STYLE_INHERITANCE_CYCLE);
    }

    // ============================================================================
    // XML Generation Tests
    // ============================================================================

    TEST_F(StyleManagerTest, GenerateStyleXML)
    {
        auto style_result = style_manager->create_paragraph_style_safe("XMLTest");
        ASSERT_TRUE(style_result.ok());
        Style* style = style_result.value();

        auto xml_result = style->to_xml_safe();
        EXPECT_TRUE(xml_result.ok());

        const std::string& xml = xml_result.value();
        EXPECT_TRUE(xml.find("XMLTest") != std::string::npos);
        EXPECT_TRUE(xml.find("w:style") != std::string::npos);
        EXPECT_TRUE(xml.find("paragraph") != std::string::npos);
    }

    TEST_F(StyleManagerTest, GenerateAllStylesXML)
    {
        style_manager->create_paragraph_style_safe("Para1");
        style_manager->create_character_style_safe("Char1");

        auto xml_result = style_manager->generate_styles_xml_safe();
        EXPECT_TRUE(xml_result.ok());

        const std::string& xml = xml_result.value();
        EXPECT_TRUE(xml.find("w:styles") != std::string::npos);
        EXPECT_TRUE(xml.find("Para1") != std::string::npos);
        EXPECT_TRUE(xml.find("Char1") != std::string::npos);
    }

    // ============================================================================
    // Validation Tests
    // ============================================================================

    TEST_F(StyleManagerTest, ValidateAllStyles)
    {
        style_manager->create_paragraph_style_safe("Valid1");
        style_manager->create_character_style_safe("Valid2");

        auto result = style_manager->validate_all_styles_safe();
        EXPECT_TRUE(result.ok()) << "Style validation failed: " << result.error().to_string();
    }

    TEST_F(StyleManagerTest, ClearAllStyles)
    {
        style_manager->create_paragraph_style_safe("Test1");
        style_manager->create_character_style_safe("Test2");
        EXPECT_EQ(style_manager->style_count(), 2);

        auto result = style_manager->clear_all_styles_safe();
        EXPECT_TRUE(result.ok());
        EXPECT_EQ(style_manager->style_count(), 0);
    }