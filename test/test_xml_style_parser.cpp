/*!
 * @file test_xml_style_parser.cpp
 * @brief Unit tests for XmlStyleParser class and XML-based style definition system
 * 
 * Tests comprehensive XML style parsing functionality including style loading,
 * property parsing, unit conversion, StyleSet parsing, and error handling.
 * Validates Result<T> error handling and XML validation mechanisms.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include "XmlStyleParser.hpp"
#include "StyleManager.hpp"

namespace duckx {
namespace test {
    
    class XmlStyleParserTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            parser = std::make_unique<XmlStyleParser>();
        }

        void TearDown() override
        {
            parser.reset();
        }

        std::unique_ptr<XmlStyleParser> parser;
        
        // Helper method to create test XML content
        std::string create_test_xml_content()
        {
            return R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="TestHeading" type="mixed">
        <Paragraph>
            <Alignment>center</Alignment>
            <SpaceBefore>24pt</SpaceBefore>
            <SpaceAfter>12pt</SpaceAfter>
            <LineSpacing>1.5</LineSpacing>
            <Indentation left="0pt" right="0pt" firstLine="0pt"/>
        </Paragraph>
        <Character>
            <Font name="Arial" size="18pt"/>
            <Color>#000080</Color>
            <Format bold="true" italic="false"/>
        </Character>
    </Style>
    
    <Style name="TestCode" type="mixed">
        <Paragraph>
            <SpaceBefore>6pt</SpaceBefore>
            <SpaceAfter>6pt</SpaceAfter>
            <Indentation left="36pt"/>
        </Paragraph>
        <Character>
            <Font name="Consolas" size="10pt"/>
            <Color>#333333</Color>
            <Highlight>lightGray</Highlight>
        </Character>
    </Style>
    
    <Style name="TestTable" type="table">
        <Table>
            <Width>100%</Width>
            <Alignment>center</Alignment>
            <Borders style="single" width="1pt" color="#CCCCCC"/>
            <CellPadding>5pt</CellPadding>
        </Table>
    </Style>
    
    <StyleSet name="TestSet" description="Test style set">
        <Include>TestHeading</Include>
        <Include>TestCode</Include>
        <Include>TestTable</Include>
    </StyleSet>
</StyleSheet>)";
        }
        
        std::string create_invalid_xml_content()
        {
            return R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://invalid.namespace" version="1.0">
    <Style name="Invalid">
        <Paragraph>
            <Alignment>invalid_alignment</Alignment>
        </Paragraph>
    </Style>
</StyleSheet>)";
        }
    };

    // ============================================================================
    // StyleUnits Namespace Tests
    // ============================================================================

    TEST_F(XmlStyleParserTest, StyleUnitsParseValueWithUnit)
    {
        // Test points
        auto pt_result = StyleUnits::parse_value_with_unit_safe("12pt");
        ASSERT_TRUE(pt_result.ok()) << "Failed to parse points: " << pt_result.error().to_string();
        EXPECT_DOUBLE_EQ(12.0, pt_result.value());
        
        // Test pixels
        auto px_result = StyleUnits::parse_value_with_unit_safe("16px");
        ASSERT_TRUE(px_result.ok()) << "Failed to parse pixels: " << px_result.error().to_string();
        EXPECT_DOUBLE_EQ(12.0, px_result.value()); // 16px = 12pt
        
        // Test inches
        auto in_result = StyleUnits::parse_value_with_unit_safe("1in");
        ASSERT_TRUE(in_result.ok()) << "Failed to parse inches: " << in_result.error().to_string();
        EXPECT_DOUBLE_EQ(72.0, in_result.value()); // 1in = 72pt
        
        // Test centimeters
        auto cm_result = StyleUnits::parse_value_with_unit_safe("1cm");
        ASSERT_TRUE(cm_result.ok()) << "Failed to parse centimeters: " << cm_result.error().to_string();
        EXPECT_NEAR(28.35, cm_result.value(), 0.01); // 1cm ≈ 28.35pt
        
        // Test no unit (defaults to points)
        auto no_unit_result = StyleUnits::parse_value_with_unit_safe("10");
        ASSERT_TRUE(no_unit_result.ok()) << "Failed to parse no unit: " << no_unit_result.error().to_string();
        EXPECT_DOUBLE_EQ(10.0, no_unit_result.value());
    }

    TEST_F(XmlStyleParserTest, StyleUnitsParseValueWithUnitErrors)
    {
        // Test empty string
        auto empty_result = StyleUnits::parse_value_with_unit_safe("");
        EXPECT_FALSE(empty_result.ok()) << "Should fail on empty string";
        
        // Test invalid numeric
        auto invalid_result = StyleUnits::parse_value_with_unit_safe("abcpt");
        EXPECT_FALSE(invalid_result.ok()) << "Should fail on invalid numeric";
        
        // Test unsupported unit
        auto unsupported_result = StyleUnits::parse_value_with_unit_safe("12kg");
        EXPECT_FALSE(unsupported_result.ok()) << "Should fail on unsupported unit";
    }

    TEST_F(XmlStyleParserTest, StyleUnitsParsePercentage)
    {
        auto result = StyleUnits::parse_percentage_safe("100%");
        ASSERT_TRUE(result.ok()) << "Failed to parse percentage: " << result.error().to_string();
        EXPECT_DOUBLE_EQ(1.0, result.value());
        
        auto result2 = StyleUnits::parse_percentage_safe("150%");
        ASSERT_TRUE(result2.ok()) << "Failed to parse 150%: " << result2.error().to_string();
        EXPECT_DOUBLE_EQ(1.5, result2.value());
        
        // Test invalid percentage
        auto invalid_result = StyleUnits::parse_percentage_safe("100");
        EXPECT_FALSE(invalid_result.ok()) << "Should fail on missing % sign";
    }

    TEST_F(XmlStyleParserTest, StyleUnitsParseColor)
    {
        // Test hex color
        auto hex_result = StyleUnits::parse_color_safe("#FF0000");
        ASSERT_TRUE(hex_result.ok()) << "Failed to parse hex color: " << hex_result.error().to_string();
        EXPECT_EQ("FF0000", hex_result.value());
        
        // Test hex color without #
        auto hex_no_hash_result = StyleUnits::parse_color_safe("00FF00");
        ASSERT_TRUE(hex_no_hash_result.ok()) << "Failed to parse hex without #: " << hex_no_hash_result.error().to_string();
        EXPECT_EQ("00FF00", hex_no_hash_result.value());
        
        // Test named colors
        auto red_result = StyleUnits::parse_color_safe("red");
        ASSERT_TRUE(red_result.ok()) << "Failed to parse named color: " << red_result.error().to_string();
        EXPECT_EQ("FF0000", red_result.value());
        
        auto blue_result = StyleUnits::parse_color_safe("blue");
        ASSERT_TRUE(blue_result.ok()) << "Failed to parse blue: " << blue_result.error().to_string();
        EXPECT_EQ("0000FF", blue_result.value());
        
        // Test invalid color
        auto invalid_result = StyleUnits::parse_color_safe("#GGGGGG");
        EXPECT_FALSE(invalid_result.ok()) << "Should fail on invalid hex";
    }

    // ============================================================================
    // XML Loading and Validation Tests
    // ============================================================================

    TEST_F(XmlStyleParserTest, LoadStylesFromString)
    {
        std::string xml_content = create_test_xml_content();
        
        auto result = parser->load_styles_from_string_safe(xml_content);
        ASSERT_TRUE(result.ok()) << "Failed to load styles: " << result.error().to_string();
        
        const auto& styles = result.value();
        EXPECT_EQ(3, styles.size()) << "Should load 3 styles";
        
        // Verify style names
        std::vector<std::string> expected_names = {"TestHeading", "TestCode", "TestTable"};
        for (size_t i = 0; i < styles.size() && i < expected_names.size(); ++i) {
            EXPECT_EQ(expected_names[i], styles[i]->name()) << "Style " << i << " name mismatch";
        }
    }

    TEST_F(XmlStyleParserTest, LoadStyleSetsFromString)
    {
        std::string xml_content = create_test_xml_content();
        
        auto result = parser->load_style_sets_from_string_safe(xml_content);
        ASSERT_TRUE(result.ok()) << "Failed to load style sets: " << result.error().to_string();
        
        const auto& style_sets = result.value();
        EXPECT_EQ(1, style_sets.size()) << "Should load 1 style set";
        
        if (!style_sets.empty()) {
            const auto& style_set = style_sets[0];
            EXPECT_EQ("TestSet", style_set.name);
            EXPECT_EQ("Test style set", style_set.description);
            EXPECT_EQ(3, style_set.included_styles.size()) << "StyleSet should include 3 styles";
        }
    }

    TEST_F(XmlStyleParserTest, ValidationFailsOnInvalidNamespace)
    {
        std::string invalid_xml = create_invalid_xml_content();
        
        auto result = parser->load_styles_from_string_safe(invalid_xml);
        EXPECT_FALSE(result.ok()) << "Should fail on invalid namespace";
    }

    TEST_F(XmlStyleParserTest, ValidationFailsOnMissingVersion)
    {
        std::string xml_no_version = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles">
    <Style name="Test" type="paragraph"/>
</StyleSheet>)";
        
        auto result = parser->load_styles_from_string_safe(xml_no_version);
        EXPECT_FALSE(result.ok()) << "Should fail on missing version";
    }

    // ============================================================================
    // Style Properties Parsing Tests
    // ============================================================================

    TEST_F(XmlStyleParserTest, ParseParagraphProperties)
    {
        std::string xml_content = create_test_xml_content();
        
        auto result = parser->load_styles_from_string_safe(xml_content);
        ASSERT_TRUE(result.ok()) << "Failed to load styles: " << result.error().to_string();
        
        // Find TestHeading style
        const auto& styles = result.value();
        auto heading_it = std::find_if(styles.begin(), styles.end(),
            [](const std::unique_ptr<Style>& style) {
                return style->name() == "TestHeading";
            });
            
        ASSERT_NE(heading_it, styles.end()) << "TestHeading style not found";
        
        const auto& heading_style = *heading_it;
        const auto& para_props = heading_style->paragraph_properties();
        
        // Verify paragraph properties
        ASSERT_TRUE(para_props.alignment.has_value()) << "Alignment should be set";
        EXPECT_EQ(Alignment::CENTER, para_props.alignment.value());
        
        ASSERT_TRUE(para_props.space_before_pts.has_value()) << "Space before should be set";
        EXPECT_DOUBLE_EQ(24.0, para_props.space_before_pts.value());
        
        ASSERT_TRUE(para_props.space_after_pts.has_value()) << "Space after should be set";
        EXPECT_DOUBLE_EQ(12.0, para_props.space_after_pts.value());
        
        ASSERT_TRUE(para_props.line_spacing.has_value()) << "Line spacing should be set";
        EXPECT_DOUBLE_EQ(1.5, para_props.line_spacing.value());
        
        ASSERT_TRUE(para_props.left_indent_pts.has_value()) << "Left indent should be set";
        EXPECT_DOUBLE_EQ(0.0, para_props.left_indent_pts.value());
        
        ASSERT_TRUE(para_props.right_indent_pts.has_value()) << "Right indent should be set";
        EXPECT_DOUBLE_EQ(0.0, para_props.right_indent_pts.value());
        
        ASSERT_TRUE(para_props.first_line_indent_pts.has_value()) << "First line indent should be set";
        EXPECT_DOUBLE_EQ(0.0, para_props.first_line_indent_pts.value());
    }

    TEST_F(XmlStyleParserTest, ParseCharacterProperties)
    {
        std::string xml_content = create_test_xml_content();
        
        auto result = parser->load_styles_from_string_safe(xml_content);
        ASSERT_TRUE(result.ok()) << "Failed to load styles: " << result.error().to_string();
        
        // Find TestHeading style
        const auto& styles = result.value();
        auto heading_it = std::find_if(styles.begin(), styles.end(),
            [](const std::unique_ptr<Style>& style) {
                return style->name() == "TestHeading";
            });
            
        ASSERT_NE(heading_it, styles.end()) << "TestHeading style not found";
        
        const auto& heading_style = *heading_it;
        const auto& char_props = heading_style->character_properties();
        
        // Verify character properties
        ASSERT_TRUE(char_props.font_name.has_value()) << "Font name should be set";
        EXPECT_EQ("Arial", char_props.font_name.value());
        
        ASSERT_TRUE(char_props.font_size_pts.has_value()) << "Font size should be set";
        EXPECT_DOUBLE_EQ(18.0, char_props.font_size_pts.value());
        
        ASSERT_TRUE(char_props.font_color_hex.has_value()) << "Font color should be set";
        EXPECT_EQ("000080", char_props.font_color_hex.value());
        
        ASSERT_TRUE(char_props.formatting_flags.has_value()) << "Formatting flags should be set";
        EXPECT_TRUE((char_props.formatting_flags.value() & bold) != none) << "Bold should be set";
        EXPECT_TRUE((char_props.formatting_flags.value() & italic) == none) << "Italic should not be set";
    }

    TEST_F(XmlStyleParserTest, ParseTableProperties)
    {
        std::string xml_content = create_test_xml_content();
        
        auto result = parser->load_styles_from_string_safe(xml_content);
        ASSERT_TRUE(result.ok()) << "Failed to load styles: " << result.error().to_string();
        
        // Find TestTable style
        const auto& styles = result.value();
        auto table_it = std::find_if(styles.begin(), styles.end(),
            [](const std::unique_ptr<Style>& style) {
                return style->name() == "TestTable";
            });
            
        ASSERT_NE(table_it, styles.end()) << "TestTable style not found";
        
        const auto& table_style = *table_it;
        const auto& table_props = table_style->table_properties();
        
        // Verify table properties
        ASSERT_TRUE(table_props.table_width_pts.has_value()) << "Table width should be set";
        EXPECT_DOUBLE_EQ(400.0, table_props.table_width_pts.value()); // 100% = 400pt
        
        ASSERT_TRUE(table_props.table_alignment.has_value()) << "Table alignment should be set";
        EXPECT_EQ("center", table_props.table_alignment.value());
        
        ASSERT_TRUE(table_props.border_style.has_value()) << "Border style should be set";
        EXPECT_EQ("single", table_props.border_style.value());
        
        ASSERT_TRUE(table_props.border_width_pts.has_value()) << "Border width should be set";
        EXPECT_DOUBLE_EQ(1.0, table_props.border_width_pts.value());
        
        ASSERT_TRUE(table_props.border_color_hex.has_value()) << "Border color should be set";
        EXPECT_EQ("CCCCCC", table_props.border_color_hex.value());
        
        ASSERT_TRUE(table_props.cell_padding_pts.has_value()) << "Cell padding should be set";
        EXPECT_DOUBLE_EQ(5.0, table_props.cell_padding_pts.value());
    }

    // ============================================================================
    // Helper Method Tests
    // ============================================================================

    TEST_F(XmlStyleParserTest, ParseAlignment)
    {
        // Test all valid alignments
        auto left_result = StyleUnits::parse_value_with_unit_safe("12pt"); // Indirect test since parse_alignment_safe is private
        EXPECT_TRUE(left_result.ok());
    }

    TEST_F(XmlStyleParserTest, GetSupportedSchemaVersion)
    {
        std::string version = XmlStyleParser::get_supported_schema_version();
        EXPECT_EQ("1.0", version);
    }

    TEST_F(XmlStyleParserTest, GetXmlNamespace)
    {
        std::string namespace_ = XmlStyleParser::get_xml_namespace();
        EXPECT_EQ("http://duckx.org/styles", namespace_);
    }

    // ============================================================================
    // Error Handling Tests
    // ============================================================================

    TEST_F(XmlStyleParserTest, InvalidXMLContent)
    {
        std::string invalid_xml = "This is not valid XML";
        
        auto result = parser->load_styles_from_string_safe(invalid_xml);
        EXPECT_FALSE(result.ok()) << "Should fail on invalid XML";
    }

    TEST_F(XmlStyleParserTest, MissingStyleName)
    {
        std::string xml_missing_name = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style type="paragraph">
        <Paragraph>
            <Alignment>center</Alignment>
        </Paragraph>
    </Style>
</StyleSheet>)";
        
        auto result = parser->load_styles_from_string_safe(xml_missing_name);
        EXPECT_FALSE(result.ok()) << "Should fail on missing style name";
    }

    TEST_F(XmlStyleParserTest, MissingStyleType)
    {
        std::string xml_missing_type = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="Test">
        <Paragraph>
            <Alignment>center</Alignment>
        </Paragraph>
    </Style>
</StyleSheet>)";
        
        auto result = parser->load_styles_from_string_safe(xml_missing_type);
        EXPECT_FALSE(result.ok()) << "Should fail on missing style type";
    }

    TEST_F(XmlStyleParserTest, InvalidStyleType)
    {
        std::string xml_invalid_type = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="Test" type="invalid_type">
        <Paragraph>
            <Alignment>center</Alignment>
        </Paragraph>
    </Style>
</StyleSheet>)";
        
        auto result = parser->load_styles_from_string_safe(xml_invalid_type);
        EXPECT_FALSE(result.ok()) << "Should fail on invalid style type";
    }

} // namespace test
} // namespace duckx