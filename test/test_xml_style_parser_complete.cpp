/*!
 * @file test_xml_style_parser_complete.cpp
 * @brief Comprehensive tests for completed XmlStyleParser
 * 
 * Tests the recently implemented private methods by testing their functionality
 * through the public XML parsing interface to verify they work correctly
 * with the existing enumeration values.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <string>

#include "XmlStyleParser.hpp"
#include "constants.hpp"
#include "test_utils.hpp"

using namespace duckx;

class XmlStyleParserCompleteTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_utils::create_directory("temp");
    }
    
    XmlStyleParser parser;
};

// ============================================================================
// Highlight Color Parsing Tests (through XML)
// ============================================================================

TEST_F(XmlStyleParserCompleteTest, ParseHighlightColor_YellowColor)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="HighlightTest" type="character">
        <Character>
            <Highlight>yellow</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    ASSERT_EQ(styles_result.value().size(), 1);
    
    const std::unique_ptr<Style>& style = styles_result.value()[0];
    const auto& char_props = style->character_properties();
    
    ASSERT_TRUE(char_props.highlight_color.has_value());
    EXPECT_EQ(char_props.highlight_color.value(), HighlightColor::YELLOW);
}

TEST_F(XmlStyleParserCompleteTest, ParseHighlightColor_RedColor)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="HighlightTest" type="character">
        <Character>
            <Highlight>red</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    ASSERT_EQ(styles_result.value().size(), 1);
    
    const std::unique_ptr<Style>& style = styles_result.value()[0];
    const auto& char_props = style->character_properties();
    
    ASSERT_TRUE(char_props.highlight_color.has_value());
    EXPECT_EQ(char_props.highlight_color.value(), HighlightColor::RED);
}

TEST_F(XmlStyleParserCompleteTest, ParseHighlightColor_LightGrayVariants)
{
    // Test lightgray
    std::string xml_content1 = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="HighlightTest1" type="character">
        <Character>
            <Highlight>lightgray</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result1 = parser.load_styles_from_string_safe(xml_content1);
    ASSERT_TRUE(styles_result1.ok());
    const auto& char_props1 = styles_result1.value()[0]->character_properties();
    ASSERT_TRUE(char_props1.highlight_color.has_value());
    EXPECT_EQ(char_props1.highlight_color.value(), HighlightColor::LIGHT_GRAY);
    
    // Test lightgrey
    std::string xml_content2 = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="HighlightTest2" type="character">
        <Character>
            <Highlight>lightgrey</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result2 = parser.load_styles_from_string_safe(xml_content2);
    ASSERT_TRUE(styles_result2.ok());
    const auto& char_props2 = styles_result2.value()[0]->character_properties();
    ASSERT_TRUE(char_props2.highlight_color.has_value());
    EXPECT_EQ(char_props2.highlight_color.value(), HighlightColor::LIGHT_GRAY);
    
    // Test light-gray
    std::string xml_content3 = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="HighlightTest3" type="character">
        <Character>
            <Highlight>light-gray</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result3 = parser.load_styles_from_string_safe(xml_content3);
    ASSERT_TRUE(styles_result3.ok());
    const auto& char_props3 = styles_result3.value()[0]->character_properties();
    ASSERT_TRUE(char_props3.highlight_color.has_value());
    EXPECT_EQ(char_props3.highlight_color.value(), HighlightColor::LIGHT_GRAY);
}

TEST_F(XmlStyleParserCompleteTest, ParseHighlightColor_DarkColors)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="DarkBlueTest" type="character">
        <Character>
            <Highlight>darkblue</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    const auto& char_props = styles_result.value()[0]->character_properties();
    ASSERT_TRUE(char_props.highlight_color.has_value());
    EXPECT_EQ(char_props.highlight_color.value(), HighlightColor::DARK_BLUE);
}

TEST_F(XmlStyleParserCompleteTest, ParseHighlightColor_InvalidColor)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="InvalidHighlightTest" type="character">
        <Character>
            <Highlight>invalid_color</Highlight>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    const auto& char_props = styles_result.value()[0]->character_properties();
    
    // Invalid colors should result in no highlight being set (nullopt)
    EXPECT_FALSE(char_props.highlight_color.has_value());
}

// ============================================================================
// Formatting Flags Parsing Tests (through XML)
// ============================================================================

TEST_F(XmlStyleParserCompleteTest, ParseFormattingFlags_SingleFlags)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="BoldTest" type="character">
        <Character>
            <Format bold="true"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    const auto& char_props = styles_result.value()[0]->character_properties();
    
    ASSERT_TRUE(char_props.formatting_flags.has_value());
    EXPECT_EQ(char_props.formatting_flags.value(), bold);
}

TEST_F(XmlStyleParserCompleteTest, ParseFormattingFlags_MultipleFlags)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="MultiFlagTest" type="character">
        <Character>
            <Format bold="true" italic="true" underline="true"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    const auto& char_props = styles_result.value()[0]->character_properties();
    
    formatting_flag expected = bold | italic | underline;
    ASSERT_TRUE(char_props.formatting_flags.has_value());
    EXPECT_EQ(char_props.formatting_flags.value(), expected);
}

TEST_F(XmlStyleParserCompleteTest, ParseFormattingFlags_VariousTrueValues)
{
    // Test with "1" value
    std::string xml_content1 = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="OneValueTest" type="character">
        <Character>
            <Format bold="1"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result1 = parser.load_styles_from_string_safe(xml_content1);
    ASSERT_TRUE(styles_result1.ok());
    const auto& char_props1 = styles_result1.value()[0]->character_properties();
    ASSERT_TRUE(char_props1.formatting_flags.has_value());
    EXPECT_EQ(char_props1.formatting_flags.value(), bold);
    
    // Test with "yes" value
    std::string xml_content2 = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="YesValueTest" type="character">
        <Character>
            <Format italic="yes"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result2 = parser.load_styles_from_string_safe(xml_content2);
    ASSERT_TRUE(styles_result2.ok());
    const auto& char_props2 = styles_result2.value()[0]->character_properties();
    ASSERT_TRUE(char_props2.formatting_flags.has_value());
    EXPECT_EQ(char_props2.formatting_flags.value(), italic);
    
    // Test with "TRUE" (case insensitive)
    std::string xml_content3 = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="TrueValueTest" type="character">
        <Character>
            <Format underline="TRUE"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result3 = parser.load_styles_from_string_safe(xml_content3);
    ASSERT_TRUE(styles_result3.ok());
    const auto& char_props3 = styles_result3.value()[0]->character_properties();
    ASSERT_TRUE(char_props3.formatting_flags.has_value());
    EXPECT_EQ(char_props3.formatting_flags.value(), underline);
}

TEST_F(XmlStyleParserCompleteTest, ParseFormattingFlags_FalseValues)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="FalseTest" type="character">
        <Character>
            <Format bold="false" italic="0" underline="no"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    const auto& char_props = styles_result.value()[0]->character_properties();
    
    // For false values, formatting_flags should either not be set or be none
    if (char_props.formatting_flags.has_value()) {
        EXPECT_EQ(char_props.formatting_flags.value(), none);
    }
}

TEST_F(XmlStyleParserCompleteTest, ParseFormattingFlags_AllSupportedFlags)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="AllFlagsTest" type="character">
        <Character>
            <Format bold="true" italic="true" underline="true" strikethrough="true" subscript="true" superscript="true" smallCaps="true" shadow="true"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    const auto& char_props = styles_result.value()[0]->character_properties();
    
    formatting_flag expected = bold | italic | underline | strikethrough | subscript | superscript | smallcaps | shadow;
    ASSERT_TRUE(char_props.formatting_flags.has_value());
    EXPECT_EQ(char_props.formatting_flags.value(), expected);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(XmlStyleParserCompleteTest, IntegrationTest_CompleteXMLParsing)
{
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="CompleteTest" type="character">
        <Character>
            <Font name="Arial" size="12pt"/>
            <Color>#FF0000</Color>
            <Highlight>yellow</Highlight>
            <Format bold="true" italic="true" underline="false"/>
        </Character>
    </Style>
</StyleSheet>)";
    
    auto styles_result = parser.load_styles_from_string_safe(xml_content);
    ASSERT_TRUE(styles_result.ok());
    ASSERT_EQ(styles_result.value().size(), 1);
    
    const std::unique_ptr<Style>& style = styles_result.value()[0];
    EXPECT_EQ(style->name(), "CompleteTest");
    EXPECT_EQ(style->type(), StyleType::CHARACTER);
    
    const auto& char_props = style->character_properties();
    
    ASSERT_TRUE(char_props.font_name.has_value());
    EXPECT_EQ(char_props.font_name.value(), "Arial");
    ASSERT_TRUE(char_props.font_size_pts.has_value());
    EXPECT_EQ(char_props.font_size_pts.value(), 12.0);
    ASSERT_TRUE(char_props.font_color_hex.has_value());
    EXPECT_EQ(char_props.font_color_hex.value(), "FF0000");
    ASSERT_TRUE(char_props.highlight_color.has_value());
    EXPECT_EQ(char_props.highlight_color.value(), HighlightColor::YELLOW);
    ASSERT_TRUE(char_props.formatting_flags.has_value());
    EXPECT_EQ(char_props.formatting_flags.value(), bold | italic);
}

// Test completed - no main function needed as this is included in unified test runner