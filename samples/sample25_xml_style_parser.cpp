/*!
 * @file sample25_xml_style_parser.cpp
 * @brief Demonstrate XML-based style definition system
 * 
 * This sample demonstrates the XML-based style definition system,
 * showing how to load styles from XML files, parse properties,
 * and work with StyleSet collections for document styling.
 */

#include <iostream>
#include <memory>
#include <fstream>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "XmlStyleParser.hpp"
#include "test_utils.hpp"

using namespace duckx;

// Helper function to create example XML content
std::string create_example_xml_content()
{
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <!-- 基础标题样式 -->
    <Style name="MyHeading1" type="mixed">
        <Paragraph>
            <Alignment>center</Alignment>
            <SpaceBefore>24pt</SpaceBefore>
            <SpaceAfter>12pt</SpaceAfter>
            <LineSpacing>1.5</LineSpacing>
            <Indentation left="0pt" right="0pt" firstLine="0pt"/>
        </Paragraph>
        <Character>
            <Font name="Arial" size="24pt"/>
            <Color>#000080</Color>
            <Format bold="true" italic="false"/>
        </Character>
    </Style>
    
    <!-- 代码块样式 -->
    <Style name="CodeBlock" type="mixed">
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
    
    <!-- 表格样式 -->
    <Style name="ModernTable" type="table">
        <Table>
            <Width>100%</Width>
            <Alignment>center</Alignment>
            <Borders style="single" width="1pt" color="#CCCCCC"/>
            <CellPadding>5pt</CellPadding>
        </Table>
    </Style>
    
    <!-- 纯段落样式（只有段落属性） -->
    <Style name="PureParagraph" type="paragraph">
        <Paragraph>
            <Alignment>left</Alignment>
            <SpaceBefore>12pt</SpaceBefore>
            <SpaceAfter>6pt</SpaceAfter>
            <Indentation left="20pt"/>
        </Paragraph>
    </Style>
    
    <!-- 纯字符样式（只有字符属性） -->
    <Style name="PureCharacter" type="character">
        <Character>
            <Font name="Times New Roman" size="14pt"/>
            <Color>#800000</Color>
            <Format bold="false" italic="true"/>
        </Character>
    </Style>
    
    <!-- 样式集合定义 -->
    <StyleSet name="TechnicalDocument" description="用于技术文档的样式集合">
        <Include>MyHeading1</Include>
        <Include>CodeBlock</Include>
        <Include>ModernTable</Include>
        <Include>PureParagraph</Include>
        <Include>PureCharacter</Include>
    </StyleSet>
</StyleSheet>)";
}

int main()
{
    try {
        std::cout << "=== XML Style Parser Demonstration ===" << std::endl;
        
        // Test 1: Create XML style parser
        std::cout << "\n--- Test 1: XML Style Parser Creation ---" << std::endl;
        XmlStyleParser parser;
        std::cout << "✓ XmlStyleParser created successfully" << std::endl;
        
        // Test 2: Create and save example XML file
        std::cout << "\n--- Test 2: Create Example XML File ---" << std::endl;
        std::string xml_file_path = duckx::test_utils::get_temp_path("example_styles.xml");
        std::string xml_content = create_example_xml_content();
        
        // Write XML content to file
        {
            std::ofstream xml_file(xml_file_path);
            if (!xml_file.is_open()) {
                std::cerr << "Failed to create XML file: " << xml_file_path << std::endl;
                return 1;
            }
            xml_file << xml_content;
            if (!xml_file.good()) {
                std::cerr << "Failed to write XML content to file: " << xml_file_path << std::endl;
                return 1;
            }
        } // File automatically closed by destructor
        std::cout << "✓ Created XML file: " << xml_file_path << std::endl;
        
        // Test 3: Load styles from XML file
        std::cout << "\n--- Test 3: Load Styles from XML ---" << std::endl;
        
        auto styles_result = parser.load_styles_from_file_safe(xml_file_path);
        if (!styles_result.ok()) {
            std::cerr << "Failed to load styles from XML: " << styles_result.error().to_string() << std::endl;
            return 1;
        }
        
        auto& styles = styles_result.value();
        std::cout << "✓ Loaded " << styles.size() << " styles from XML file" << std::endl;
        
        // Display loaded styles
        for (const auto& style : styles) {
            std::cout << "  - Style: " << style->name() << " (type: " << static_cast<int>(style->type()) << ")" << std::endl;
        }
        
        // Test 4: Load style sets from XML
        std::cout << "\n--- Test 4: Load Style Sets from XML ---" << std::endl;
        auto style_sets_result = parser.load_style_sets_from_file_safe(xml_file_path);
        if (!style_sets_result.ok()) {
            std::cerr << "Failed to load style sets: " << style_sets_result.error().to_string() << std::endl;
            return 1;
        }
        
        auto& style_sets = style_sets_result.value();
        std::cout << "✓ Loaded " << style_sets.size() << " style sets from XML file" << std::endl;
        
        // Display loaded style sets
        for (const auto& style_set : style_sets) {
            std::cout << "  - StyleSet: " << style_set.name;
            if (!style_set.description.empty()) {
                std::cout << " (" << style_set.description << ")";
            }
            std::cout << std::endl;
            std::cout << "    Includes " << style_set.included_styles.size() << " styles: ";
            for (size_t i = 0; i < style_set.included_styles.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << style_set.included_styles[i];
            }
            std::cout << std::endl;
        }
        
        // Test 5: Test style properties parsing
        std::cout << "\n--- Test 5: Verify Style Properties ---" << std::endl;
        
        // Find and examine the MyHeading1 style
        auto heading_style_it = std::find_if(styles.begin(), styles.end(),
            [](const std::unique_ptr<Style>& style) {
                return style->name() == "MyHeading1";
            });
            
        if (heading_style_it != styles.end()) {
            const auto& heading_style = *heading_style_it;
            const auto& para_props = heading_style->paragraph_properties();
            const auto& char_props = heading_style->character_properties();
            
            std::cout << "✓ Found MyHeading1 style:" << std::endl;
            
            // Check paragraph properties
            if (para_props.alignment.has_value()) {
                std::cout << "  - Alignment: " << static_cast<int>(para_props.alignment.value()) << std::endl;
            }
            if (para_props.space_before_pts.has_value()) {
                std::cout << "  - Space before: " << para_props.space_before_pts.value() << " pts" << std::endl;
            }
            if (para_props.space_after_pts.has_value()) {
                std::cout << "  - Space after: " << para_props.space_after_pts.value() << " pts" << std::endl;
            }
            if (para_props.line_spacing.has_value()) {
                std::cout << "  - Line spacing: " << para_props.line_spacing.value() << std::endl;
            }
            
            // Check character properties
            if (char_props.font_name.has_value()) {
                std::cout << "  - Font name: " << char_props.font_name.value() << std::endl;
            }
            if (char_props.font_size_pts.has_value()) {
                std::cout << "  - Font size: " << char_props.font_size_pts.value() << " pts" << std::endl;
            }
            if (char_props.font_color_hex.has_value()) {
                std::cout << "  - Font color: #" << char_props.font_color_hex.value() << std::endl;
            }
            if (char_props.formatting_flags.has_value()) {
                std::cout << "  - Formatting flags: " << static_cast<int>(char_props.formatting_flags.value()) << std::endl;
            }
        }
        
        // Test 6: Test loading from string (alternative approach)
        std::cout << "\n--- Test 6: Load Styles from String (Alternative) ---" << std::endl;
        
        auto string_styles_result = parser.load_styles_from_string_safe(xml_content);
        if (string_styles_result.ok()) {
            std::cout << "✓ Also loaded " << string_styles_result.value().size() 
                      << " styles from XML string content" << std::endl;
        } else {
            std::cerr << "Failed to load from string: " << string_styles_result.error().to_string() << std::endl;
        }
        
        // Test 7: Test unit conversion
        std::cout << "\n--- Test 7: Unit Conversion Tests ---" << std::endl;
        
        auto pt_result = StyleUnits::parse_value_with_unit_safe("12pt");
        if (pt_result.ok()) {
            std::cout << "✓ 12pt = " << pt_result.value() << " points" << std::endl;
        }
        
        auto px_result = StyleUnits::parse_value_with_unit_safe("16px");
        if (px_result.ok()) {
            std::cout << "✓ 16px = " << px_result.value() << " points" << std::endl;
        }
        
        auto in_result = StyleUnits::parse_value_with_unit_safe("1in");
        if (in_result.ok()) {
            std::cout << "✓ 1in = " << in_result.value() << " points" << std::endl;
        }
        
        auto percent_result = StyleUnits::parse_percentage_safe("100%");
        if (percent_result.ok()) {
            std::cout << "✓ 100% = " << percent_result.value() << " (decimal)" << std::endl;
        }
        
        auto color_result = StyleUnits::parse_color_safe("#FF0000");
        if (color_result.ok()) {
            std::cout << "✓ #FF0000 = " << color_result.value() << " (hex)" << std::endl;
        }
        
        auto named_color_result = StyleUnits::parse_color_safe("blue");
        if (named_color_result.ok()) {
            std::cout << "✓ blue = " << named_color_result.value() << " (hex)" << std::endl;
        }
        
        std::cout << "\n=== All Tests Completed Successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}