/*!
 * @file sample24_style_system_test.cpp
 * @brief Test the complete style system with corrected API usage
 * 
 * This sample demonstrates the complete style system functionality
 * using the actual available APIs instead of non-existent _safe methods.
 */

#include <iostream>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"
#include "test_utils.hpp"

using namespace duckx;

int main()
{
    try {
        std::cout << "=== Style System Test ===" << std::endl;
        
        // Create document
        auto doc_result = Document::create_safe(test_utils::get_temp_path("sample24_style_system_test.docx"));
        if (!doc_result.ok()) {
            std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
            return 1;
        }
        
        Document doc = std::move(doc_result.value());
        Body& body = doc.body();
        StyleManager style_manager;
        
        // Load built-in styles
        auto load_result = style_manager.load_all_built_in_styles_safe();
        if (!load_result.ok()) {
            std::cerr << "Failed to load built-in styles: " << load_result.error().to_string() << std::endl;
            return 1;
        }
        
        std::cout << "✓ Loaded built-in styles successfully" << std::endl;
        
        // Test 1: Paragraph formatting and reading
        std::cout << "\n--- Test 1: Paragraph Formatting ---" << std::endl;
        
        auto para_result = body.add_paragraph_safe("This is a test paragraph");
        if (!para_result.ok()) {
            std::cerr << "Failed to add paragraph: " << para_result.error().to_string() << std::endl;
            return 1;
        }
        
        Paragraph& para = para_result.value();
        
        // Apply direct formatting using existing methods
        para.set_alignment(Alignment::CENTER);
        para.set_spacing(12.0, 6.0);  // before, after
        para.set_indentation(20.0, 10.0);  // left, right
        
        std::cout << "✓ Applied paragraph formatting" << std::endl;
        
        // Read properties back
        auto props_result = style_manager.read_paragraph_properties_safe(para);
        if (props_result.ok()) {
            const auto& props = props_result.value();
            std::cout << "✓ Read paragraph properties:" << std::endl;
            
            if (props.alignment.has_value()) {
                std::cout << "  - Alignment: " << static_cast<int>(props.alignment.value()) << std::endl;
            }
            if (props.space_before_pts.has_value()) {
                std::cout << "  - Space before: " << props.space_before_pts.value() << " pts" << std::endl;
            }
            if (props.space_after_pts.has_value()) {
                std::cout << "  - Space after: " << props.space_after_pts.value() << " pts" << std::endl;
            }
        } else {
            std::cerr << "Failed to read paragraph properties: " << props_result.error().to_string() << std::endl;
        }
        
        // Test 2: Character formatting and reading
        std::cout << "\n--- Test 2: Character Formatting ---" << std::endl;
        
        auto para2_result = body.add_paragraph_safe("");
        if (!para2_result.ok()) {
            std::cerr << "Failed to add second paragraph: " << para2_result.error().to_string() << std::endl;
            return 1;
        }
        
        Paragraph& para2 = para2_result.value();
        
        // Create run with formatting using existing API
        Run& run = para2.add_run("Formatted text", bold | italic);
        run.set_font("Arial").set_font_size(16.0).set_color("0000FF");
        
        std::cout << "✓ Applied character formatting" << std::endl;
        
        // Read character properties
        auto char_props_result = style_manager.read_character_properties_safe(run);
        if (char_props_result.ok()) {
            const auto& char_props = char_props_result.value();
            std::cout << "✓ Read character properties:" << std::endl;
            
            if (char_props.font_name.has_value()) {
                std::cout << "  - Font: " << char_props.font_name.value() << std::endl;
            }
            if (char_props.font_size_pts.has_value()) {
                std::cout << "  - Size: " << char_props.font_size_pts.value() << " pts" << std::endl;
            }
            if (char_props.font_color_hex.has_value()) {
                std::cout << "  - Color: " << char_props.font_color_hex.value() << std::endl;
            }
            if (char_props.formatting_flags.has_value()) {
                formatting_flag flags = char_props.formatting_flags.value();
                std::cout << "  - Bold: " << ((flags & bold) ? "Yes" : "No") << std::endl;
                std::cout << "  - Italic: " << ((flags & italic) ? "Yes" : "No") << std::endl;
            }
        } else {
            std::cerr << "Failed to read character properties: " << char_props_result.error().to_string() << std::endl;
        }
        
        // Test 3: Table formatting and reading
        std::cout << "\n--- Test 3: Table Formatting ---" << std::endl;
        
        auto table_result = body.add_table_safe(2, 3);
        if (!table_result.ok()) {
            std::cerr << "Failed to add table: " << table_result.error().to_string() << std::endl;
            return 1;
        }
        
        Table& table = table_result.value();
        
        // Apply table formatting using existing methods
        table.set_width(500.0);
        table.set_alignment("center");
        table.set_border_style("single");
        table.set_border_width(1.0);
        table.set_border_color("000000");
        
        std::cout << "✓ Applied table formatting" << std::endl;
        
        // Read table properties
        auto table_props_result = style_manager.read_table_properties_safe(table);
        if (table_props_result.ok()) {
            const auto& table_props = table_props_result.value();
            std::cout << "✓ Read table properties:" << std::endl;
            
            if (table_props.table_width_pts.has_value()) {
                std::cout << "  - Width: " << table_props.table_width_pts.value() << " pts" << std::endl;
            }
            if (table_props.table_alignment.has_value()) {
                std::cout << "  - Alignment: " << table_props.table_alignment.value() << std::endl;
            }
            if (table_props.border_style.has_value()) {
                std::cout << "  - Border style: " << table_props.border_style.value() << std::endl;
            }
        } else {
            std::cerr << "Failed to read table properties: " << table_props_result.error().to_string() << std::endl;
        }
        
        // Test 4: Style creation and application
        std::cout << "\n--- Test 4: Style Creation and Application ---" << std::endl;
        
        // Create a custom style
        auto custom_style_result = style_manager.create_mixed_style_safe("Custom Test Style");
        if (!custom_style_result.ok()) {
            std::cerr << "Failed to create custom style: " << custom_style_result.error().to_string() << std::endl;
            return 1;
        }
        
        Style* custom_style = custom_style_result.value();
        
        // Configure the style
        ParagraphStyleProperties para_props;
        para_props.alignment = Alignment::RIGHT;
        para_props.space_before_pts = 15.0;
        
        CharacterStyleProperties char_props;
        char_props.font_name = "Times New Roman";
        char_props.font_size_pts = 14.0;
        char_props.font_color_hex = "800000";
        
        auto set_para_result = custom_style->set_paragraph_properties_safe(para_props);
        auto set_char_result = custom_style->set_character_properties_safe(char_props);
        
        if (set_para_result.ok() && set_char_result.ok()) {
            std::cout << "✓ Created and configured custom style" << std::endl;
            
            // Apply properties using StyleManager
            auto para3_result = body.add_paragraph_safe("Style applied paragraph");
            if (para3_result.ok()) {
                Paragraph& para3 = para3_result.value();
                
                auto apply_para_result = style_manager.apply_paragraph_properties_safe(para3, para_props);
                if (apply_para_result.ok()) {
                    std::cout << "✓ Applied custom paragraph properties" << std::endl;
                } else {
                    std::cerr << "Failed to apply paragraph properties: " << apply_para_result.error().to_string() << std::endl;
                }
                
                Run& styled_run = para3.add_run(" with styled text");
                auto apply_char_result = style_manager.apply_character_properties_safe(styled_run, char_props);
                if (apply_char_result.ok()) {
                    std::cout << "✓ Applied custom character properties" << std::endl;
                } else {
                    std::cerr << "Failed to apply character properties: " << apply_char_result.error().to_string() << std::endl;
                }
            }
        } else {
            std::cerr << "Failed to configure custom style" << std::endl;
        }
        
        // Save the document
        auto save_result = doc.save_safe();
        if (save_result.ok()) {
            std::cout << "\n✓ Document saved successfully" << std::endl;
        } else {
            std::cerr << "Failed to save document: " << save_result.error().to_string() << std::endl;
        }
        
        std::cout << "\n=== Style System Test Completed Successfully ===" << std::endl;
        std::cout << "Total styles in manager: " << style_manager.style_count() << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
}