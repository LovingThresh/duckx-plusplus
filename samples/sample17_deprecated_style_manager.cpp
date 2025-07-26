/*!
 * @file sample17_deprecated_style_manager.cpp
 * @brief [DEPRECATED] StyleManager basic demonstration
 * 
 * NOTE: This file is deprecated. Use sample20_complete_style_system.cpp for complete style application demo.
 * 
 * Demonstrates the comprehensive style management system including:
 * - Creating custom styles (paragraph, character, table)
 * - Loading built-in style libraries
 * - Setting style properties and inheritance
 * - XML generation for DOCX integration
 * - Error handling with Result<T> pattern
 * 
 * @author DuckX-PLusPlus Development Team
 * @date 2025
 */

#include <iostream>
#include <iomanip>
#include <vector>

#include "duckx.hpp"
#include "test_utils.hpp"

using namespace duckx;

// ============================================================================
// Helper Functions for Output
// ============================================================================

void print_header(const std::string& title)
{
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void print_subheader(const std::string& title)
{
    std::cout << "\n--- " << title << " ---" << std::endl;
}

void print_success(const std::string& message)
{
    std::cout << "✓ " << message << std::endl;
}

void print_error(const std::string& message, const Error& error)
{
    std::cout << "✗ " << message << ": " << error.to_string() << std::endl;
}

void print_style_info(const Style& style)
{
    std::cout << "  Style: " << style.name()
              << " (Type: ";
    
    switch (style.type()) {
        case StyleType::PARAGRAPH: std::cout << "Paragraph"; break;
        case StyleType::CHARACTER: std::cout << "Character"; break;
        case StyleType::TABLE: std::cout << "Table"; break;
        case StyleType::MIXED: std::cout << "Mixed"; break;
        default: std::cout << "Unknown"; break;
    }
    
    std::cout << ", Built-in: " << (style.is_built_in() ? "Yes" : "No") << ")" << std::endl;
    
    if (style.base_style().has_value()) {
        std::cout << "    Base Style: " << style.base_style().value() << std::endl;
    }
}

// ============================================================================
// Style Creation Demonstrations
// ============================================================================

void demonstrate_style_creation(StyleManager& style_manager)
{
    print_header("Style Creation Demonstration");

    print_subheader("Creating Custom Mixed Styles");
    
    // Create a custom heading style that can have both paragraph and character properties
    auto heading_result = style_manager.create_mixed_style_safe("Custom Heading");
    if (heading_result.ok()) {
        Style* heading = heading_result.value();
        print_success("Created Custom Heading style");
        
        // Set paragraph properties
        ParagraphStyleProperties para_props;
        para_props.alignment = Alignment::CENTER;
        para_props.space_before_pts = 18.0;
        para_props.space_after_pts = 12.0;
        para_props.line_spacing = 1.2;
        
        auto props_result = heading->set_paragraph_properties_safe(para_props);
        if (props_result.ok()) {
            print_success("Set paragraph properties (centered, spacing)");
        } else {
            print_error("Failed to set paragraph properties", props_result.error());
        }
        
        // Set character properties using convenience method
        auto font_result = heading->set_font_safe("Arial", 18.0);
        if (font_result.ok()) {
            print_success("Set font to Arial 18pt using convenience method");
        } else {
            print_error("Failed to set font", font_result.error());
        }
        
        auto color_result = heading->set_color_safe("#2E75B6");
        if (color_result.ok()) {
            print_success("Set color to blue (#2E75B6)");
        } else {
            print_error("Failed to set color", color_result.error());
        }
        
        print_style_info(*heading);
    } else {
        print_error("Failed to create Custom Heading style", heading_result.error());
    }

    print_subheader("Creating Custom Character Styles");
    
    // Create emphasis style
    auto emphasis_result = style_manager.create_character_style_safe("Emphasis");
    if (emphasis_result.ok()) {
        Style* emphasis = emphasis_result.value();
        print_success("Created Emphasis character style");
        
        CharacterStyleProperties char_props;
        char_props.font_name = "Times New Roman";
        char_props.font_size_pts = 12.0;
        char_props.font_color_hex = "8B0000";  // Dark red
        char_props.formatting_flags = italic | bold;
        
        auto props_result = emphasis->set_character_properties_safe(char_props);
        if (props_result.ok()) {
            print_success("Set character properties (Times New Roman, 12pt, dark red, bold italic)");
        } else {
            print_error("Failed to set character properties", props_result.error());
        }
        
        print_style_info(*emphasis);
    } else {
        print_error("Failed to create Emphasis style", emphasis_result.error());
    }

    print_subheader("Creating Custom Table Style");
    
    auto table_result = style_manager.create_table_style_safe("Professional Table");
    if (table_result.ok()) {
        Style* table_style = table_result.value();
        print_success("Created Professional Table style");
        
        TableStyleProperties table_props;
        table_props.border_style = "single";
        table_props.border_width_pts = 1.0;
        table_props.border_color_hex = "333333";
        table_props.cell_padding_pts = 6.0;
        table_props.table_alignment = "center";
        
        auto props_result = table_style->set_table_properties_safe(table_props);
        if (props_result.ok()) {
            print_success("Set table properties (single border, centered)");
        } else {
            print_error("Failed to set table properties", props_result.error());
        }
        
        print_style_info(*table_style);
    } else {
        print_error("Failed to create Professional Table style", table_result.error());
    }
}

// ============================================================================
// Built-in Style Demonstrations
// ============================================================================

void demonstrate_builtin_styles(StyleManager& style_manager)
{
    print_header("Built-in Style Library Demonstration");

    print_subheader("Loading Heading Styles");
    auto heading_result = style_manager.load_built_in_styles_safe(BuiltInStyleCategory::HEADING);
    if (heading_result.ok()) {
        print_success("Loaded built-in heading styles");
        
        // Show available heading styles
        auto heading_names = style_manager.get_built_in_style_names(BuiltInStyleCategory::HEADING);
        std::cout << "  Available heading styles: ";
        for (size_t i = 0; i < heading_names.size(); ++i) {
            std::cout << heading_names[i];
            if (i < heading_names.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    } else {
        print_error("Failed to load heading styles", heading_result.error());
    }

    print_subheader("Loading Body Text Styles");
    auto body_result = style_manager.load_built_in_styles_safe(BuiltInStyleCategory::BODY_TEXT);
    if (body_result.ok()) {
        print_success("Loaded built-in body text styles");
    } else {
        print_error("Failed to load body text styles", body_result.error());
    }

    print_subheader("Loading Technical Styles");
    auto tech_result = style_manager.load_built_in_styles_safe(BuiltInStyleCategory::TECHNICAL);
    if (tech_result.ok()) {
        print_success("Loaded built-in technical styles");
        
        // Demonstrate getting a specific built-in style
        auto code_result = style_manager.get_style_safe("Code");
        if (code_result.ok()) {
            print_success("Retrieved Code style");
            print_style_info(*code_result.value());
        } else {
            print_error("Failed to retrieve Code style", code_result.error());
        }
    } else {
        print_error("Failed to load technical styles", tech_result.error());
    }

    print_subheader("Style Inventory");
    std::cout << "Total styles in manager: " << style_manager.style_count() << std::endl;
    
    auto all_names = style_manager.get_all_style_names();
    std::cout << "All registered styles:" << std::endl;
    for (const auto& name : all_names) {
        auto style_result = style_manager.get_style_safe(name);
        if (style_result.ok()) {
            print_style_info(*style_result.value());
        }
    }
}

// ============================================================================
// Style Inheritance Demonstration
// ============================================================================

void demonstrate_style_inheritance(StyleManager& style_manager)
{
    print_header("Style Inheritance Demonstration");

    // Create a base style that can have both paragraph and character properties
    auto base_result = style_manager.create_mixed_style_safe("Base Paragraph");
    if (base_result.ok()) {
        Style* base = base_result.value();
        print_success("Created Base Paragraph style");
        
        // Set base properties
        ParagraphStyleProperties base_props;
        base_props.alignment = Alignment::LEFT;
        base_props.space_after_pts = 6.0;
        base_props.line_spacing = 1.15;
        
        CharacterStyleProperties base_char;
        base_char.font_name = "Calibri";
        base_char.font_size_pts = 11.0;
        
        base->set_paragraph_properties_safe(base_props);
        base->set_character_properties_safe(base_char);
        print_success("Set base style properties");
    } else {
        print_error("Failed to create base style", base_result.error());
        return;
    }

    // Create derived style
    auto derived_result = style_manager.create_paragraph_style_safe("Derived Paragraph");
    if (derived_result.ok()) {
        Style* derived = derived_result.value();
        print_success("Created Derived Paragraph style");
        
        // Set inheritance
        auto inherit_result = derived->set_base_style_safe("Base Paragraph");
        if (inherit_result.ok()) {
            print_success("Set inheritance: Derived Paragraph → Base Paragraph");
        } else {
            print_error("Failed to set inheritance", inherit_result.error());
        }
        
        // Override some properties
        auto spacing_result = derived->set_spacing_safe(12.0, 12.0);
        if (spacing_result.ok()) {
            print_success("Override spacing in derived style");
        } else {
            print_error("Failed to override spacing", spacing_result.error());
        }
        
        print_style_info(*derived);
    } else {
        print_error("Failed to create derived style", derived_result.error());
    }

    print_subheader("Testing Inheritance Validation");
    
    // Try to create circular inheritance (should fail)
    auto circular_result = style_manager.get_style_safe("Base Paragraph");
    if (circular_result.ok()) {
        auto self_inherit = circular_result.value()->set_base_style_safe("Base Paragraph");
        if (!self_inherit.ok()) {
            print_success("Correctly prevented self-inheritance");
        } else {
            print_error("Failed to prevent self-inheritance", Error{});
        }
    }
}

// ============================================================================
// Error Handling Demonstration
// ============================================================================

void demonstrate_error_handling(StyleManager& style_manager)
{
    print_header("Error Handling Demonstration");

    print_subheader("Testing Invalid Operations");

    // Try to create style with empty name
    auto empty_name_result = style_manager.create_paragraph_style_safe("");
    if (!empty_name_result.ok()) {
        print_success("Correctly rejected empty style name");
        std::cout << "  Error: " << empty_name_result.error().to_string() << std::endl;
    } else {
        print_error("Failed to reject empty style name", Error{});
    }

    // Try to set invalid font size
    auto style_result = style_manager.create_character_style_safe("ErrorTest");
    if (style_result.ok()) {
        auto invalid_font = style_result.value()->set_font_safe("Arial", -10.0);
        if (!invalid_font.ok()) {
            print_success("Correctly rejected negative font size");
            std::cout << "  Error: " << invalid_font.error().to_string() << std::endl;
        } else {
            print_error("Failed to reject negative font size", Error{});
        }
    }

    // Try to set wrong property type
    auto para_style_result = style_manager.create_paragraph_style_safe("ParaOnly");
    if (para_style_result.ok()) {
        CharacterStyleProperties char_props;
        char_props.font_name = "Arial";
        
        // This should fail because paragraph styles don't accept character properties directly
        auto wrong_type = para_style_result.value()->set_character_properties_safe(char_props);
        if (!wrong_type.ok()) {
            print_success("Correctly rejected character properties on paragraph-only style");
            std::cout << "  Error: " << wrong_type.error().to_string() << std::endl;
        } else {
            print_error("Failed to reject wrong property type", Error{});
        }
    }

    // Try to get non-existent style
    auto missing_result = style_manager.get_style_safe("DoesNotExist");
    if (!missing_result.ok()) {
        print_success("Correctly handled missing style request");
        std::cout << "  Error: " << missing_result.error().to_string() << std::endl;
    } else {
        print_error("Failed to handle missing style", Error{});
    }
}

// ============================================================================
// XML Generation Demonstration
// ============================================================================

void demonstrate_xml_generation(StyleManager& style_manager)
{
    print_header("XML Generation Demonstration");

    print_subheader("Individual Style XML");
    
    auto style_result = style_manager.get_style_safe("Custom Heading");
    if (style_result.ok()) {
        auto xml_result = style_result.value()->to_xml_safe();
        if (xml_result.ok()) {
            print_success("Generated XML for Custom Heading style");
            std::cout << "Sample XML snippet:" << std::endl;
            std::cout << xml_result.value().substr(0, 200) << "..." << std::endl;
        } else {
            print_error("Failed to generate style XML", xml_result.error());
        }
    }

    print_subheader("Complete Styles Document XML");
    
    auto full_xml_result = style_manager.generate_styles_xml_safe();
    if (full_xml_result.ok()) {
        print_success("Generated complete styles.xml document");
        const std::string& xml = full_xml_result.value();
        std::cout << "XML document size: " << xml.length() << " characters" << std::endl;
        std::cout << "Contains " << style_manager.style_count() << " style definitions" << std::endl;
        
        // Show first few lines
        std::cout << "\nFirst few lines of XML:" << std::endl;
        std::istringstream iss(xml);
        std::string line;
        int line_count = 0;
        while (std::getline(iss, line) && line_count < 5) {
            std::cout << "  " << line << std::endl;
            ++line_count;
        }
    } else {
        print_error("Failed to generate complete XML", full_xml_result.error());
    }
}

// ============================================================================
// Style Management Operations
// ============================================================================

void demonstrate_style_management(StyleManager& style_manager)
{
    print_header("Style Management Operations");

    print_subheader("Style Filtering and Queries");
    
    auto para_styles = style_manager.get_style_names_by_type(StyleType::PARAGRAPH);
    std::cout << "Paragraph styles (" << para_styles.size() << "): ";
    for (size_t i = 0; i < para_styles.size() && i < 3; ++i) {
        std::cout << para_styles[i];
        if (i < para_styles.size() - 1 && i < 2) std::cout << ", ";
    }
    if (para_styles.size() > 3) std::cout << "...";
    std::cout << std::endl;

    auto char_styles = style_manager.get_style_names_by_type(StyleType::CHARACTER);
    std::cout << "Character styles (" << char_styles.size() << "): ";
    for (size_t i = 0; i < char_styles.size() && i < 3; ++i) {
        std::cout << char_styles[i];
        if (i < char_styles.size() - 1 && i < 2) std::cout << ", ";
    }
    if (char_styles.size() > 3) std::cout << "...";
    std::cout << std::endl;

    print_subheader("Style Validation");
    auto validation_result = style_manager.validate_all_styles_safe();
    if (validation_result.ok()) {
        print_success("All styles passed validation");
    } else {
        print_error("Style validation failed", validation_result.error());
    }

    print_subheader("Final Statistics");
    std::cout << "Total styles managed: " << style_manager.style_count() << std::endl;
    
    int built_in_count = 0;
    int custom_count = 0;
    auto all_names = style_manager.get_all_style_names();
    for (const auto& name : all_names) {
        auto style_result = style_manager.get_style_safe(name);
        if (style_result.ok()) {
            if (style_result.value()->is_built_in()) {
                built_in_count++;
            } else {
                custom_count++;
            }
        }
    }
    
    std::cout << "Built-in styles: " << built_in_count << std::endl;
    std::cout << "Custom styles: " << custom_count << std::endl;
}

// ============================================================================
// Main Program
// ============================================================================

int main()
{
    try {
        print_header("DuckX-PLusPlus StyleManager Demonstration");
        std::cout << "This program demonstrates the comprehensive style management system" << std::endl;
        std::cout << "including creation, inheritance, built-in styles, and error handling." << std::endl;

        // Create style manager
        StyleManager style_manager;
        std::cout << "\n✓ StyleManager created successfully" << std::endl;

        // Run demonstrations
        demonstrate_style_creation(style_manager);
        demonstrate_builtin_styles(style_manager);
        demonstrate_style_inheritance(style_manager);
        demonstrate_error_handling(style_manager);
        demonstrate_xml_generation(style_manager);
        demonstrate_style_management(style_manager);

        print_header("Demonstration Complete");
        std::cout << "✓ All StyleManager features demonstrated successfully!" << std::endl;
        std::cout << "The StyleManager provides a robust foundation for document styling" << std::endl;
        std::cout << "with modern error handling and comprehensive built-in style libraries." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}