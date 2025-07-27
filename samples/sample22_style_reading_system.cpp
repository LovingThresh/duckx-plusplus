/*!
 * @file sample22_style_reading_system.cpp
 * @brief Style reading and extraction system demonstration
 * 
 * Demonstrates reading existing styles from DOCX documents,
 * extracting style properties, analyzing document formatting,
 * and handling style inheritance with Result<T> error patterns.
 * 
 * @date 2025.07
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"
#include "test_utils.hpp"

using namespace duckx;

// ============================================================================
// Utility Functions for Pretty Output
// ============================================================================

void print_header(const std::string& title)
{
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void print_success(const std::string& message)
{
    std::cout << "✅ " << message << std::endl;
}

void print_error(const std::string& operation, const Error& error)
{
    std::cout << "❌ " << operation << " failed: " << error.to_string() << std::endl;
}

void print_paragraph_properties(const ParagraphStyleProperties& props, const std::string& prefix = "")
{
    std::cout << prefix << "Paragraph Properties:" << std::endl;
    
    if (props.alignment.has_value()) {
        std::string align_str;
        switch (props.alignment.value()) {
            case Alignment::LEFT: align_str = "left"; break;
            case Alignment::CENTER: align_str = "center"; break;
            case Alignment::RIGHT: align_str = "right"; break;
            case Alignment::BOTH: align_str = "justify"; break;
        }
        std::cout << prefix << "  - Alignment: " << align_str << std::endl;
    }
    
    if (props.space_before_pts.has_value()) {
        std::cout << prefix << "  - Space Before: " << props.space_before_pts.value() << " pts" << std::endl;
    }
    
    if (props.space_after_pts.has_value()) {
        std::cout << prefix << "  - Space After: " << props.space_after_pts.value() << " pts" << std::endl;
    }
    
    if (props.line_spacing.has_value()) {
        std::cout << prefix << "  - Line Spacing: " << props.line_spacing.value() << std::endl;
    }
    
    if (props.left_indent_pts.has_value()) {
        std::cout << prefix << "  - Left Indent: " << props.left_indent_pts.value() << " pts" << std::endl;
    }
    
    if (props.right_indent_pts.has_value()) {
        std::cout << prefix << "  - Right Indent: " << props.right_indent_pts.value() << " pts" << std::endl;
    }
    
    if (props.first_line_indent_pts.has_value()) {
        std::cout << prefix << "  - First Line Indent: " << props.first_line_indent_pts.value() << " pts" << std::endl;
    }
    
    if (props.list_type.has_value()) {
        std::cout << prefix << "  - List Type: " << static_cast<int>(props.list_type.value()) << std::endl;
    }
    
    if (props.list_level.has_value()) {
        std::cout << prefix << "  - List Level: " << props.list_level.value() << std::endl;
    }
}

void print_character_properties(const CharacterStyleProperties& props, const std::string& prefix = "")
{
    std::cout << prefix << "Character Properties:" << std::endl;
    
    if (props.font_name.has_value()) {
        std::cout << prefix << "  - Font Name: " << props.font_name.value() << std::endl;
    }
    
    if (props.font_size_pts.has_value()) {
        std::cout << prefix << "  - Font Size: " << props.font_size_pts.value() << " pts" << std::endl;
    }
    
    if (props.font_color_hex.has_value()) {
        std::cout << prefix << "  - Font Color: #" << props.font_color_hex.value() << std::endl;
    }
    
    if (props.highlight_color.has_value()) {
        std::cout << prefix << "  - Highlight Color: " << static_cast<int>(props.highlight_color.value()) << std::endl;
    }
    
    if (props.formatting_flags.has_value()) {
        std::cout << prefix << "  - Formatting: ";
        formatting_flag flags = props.formatting_flags.value();
        if (flags & bold) std::cout << "Bold ";
        if (flags & italic) std::cout << "Italic ";
        if (flags & underline) std::cout << "Underline ";
        if (flags & strikethrough) std::cout << "Strikethrough ";
        if (flags & smallcaps) std::cout << "SmallCaps ";
        std::cout << std::endl;
    }
}

void print_table_properties(const TableStyleProperties& props, const std::string& prefix = "")
{
    std::cout << prefix << "Table Properties:" << std::endl;
    
    if (props.border_style.has_value()) {
        std::cout << prefix << "  - Border Style: " << props.border_style.value() << std::endl;
    }
    
    if (props.border_width_pts.has_value()) {
        std::cout << prefix << "  - Border Width: " << props.border_width_pts.value() << " pts" << std::endl;
    }
    
    if (props.border_color_hex.has_value()) {
        std::cout << prefix << "  - Border Color: #" << props.border_color_hex.value() << std::endl;
    }
    
    if (props.cell_padding_pts.has_value()) {
        std::cout << prefix << "  - Cell Padding: " << props.cell_padding_pts.value() << " pts" << std::endl;
    }
    
    if (props.table_width_pts.has_value()) {
        std::cout << prefix << "  - Table Width: " << props.table_width_pts.value() << " pts" << std::endl;
    }
    
    if (props.table_alignment.has_value()) {
        std::cout << prefix << "  - Table Alignment: " << props.table_alignment.value() << std::endl;
    }
}

// ============================================================================
// Style Reading Demonstrations
// ============================================================================

void demonstrate_paragraph_style_reading(Body& body, StyleManager& style_manager)
{
    print_header("Paragraph Style Reading Demonstration");
    
    // Create paragraphs with different formatting
    
    // 1. Plain paragraph
    auto plain_result = body.add_paragraph_safe("This is a plain paragraph with no special formatting.");
    if (plain_result.ok()) {
        Paragraph* plain_para = &plain_result.value();
        
        auto props_result = style_manager.read_paragraph_properties_safe(*plain_para);
        if (props_result.ok()) {
            std::cout << "\n📝 Plain Paragraph:" << std::endl;
            print_paragraph_properties(props_result.value(), "  ");
            if (!props_result.value().alignment.has_value()) {
                std::cout << "  - No custom properties (uses default styling)" << std::endl;
            }
        }
    }
    
    // 2. Paragraph with applied style
    auto styled_result = body.add_paragraph_safe("This paragraph uses the built-in Heading 1 style.");
    if (styled_result.ok()) {
        Paragraph* styled_para = &styled_result.value();
        
        auto apply_result = styled_para->apply_style_safe(style_manager, "Heading 1");
        if (apply_result.ok()) {
            print_success("Applied Heading 1 style to paragraph");
            
            // Read direct properties
            auto direct_props_result = style_manager.read_paragraph_properties_safe(*styled_para);
            if (direct_props_result.ok()) {
                std::cout << "\n📝 Direct Properties (Heading 1):" << std::endl;
                print_paragraph_properties(direct_props_result.value(), "  ");
            }
            
            // Read effective properties (with inheritance)
            auto effective_props_result = style_manager.get_effective_paragraph_properties_safe(*styled_para);
            if (effective_props_result.ok()) {
                std::cout << "\n📝 Effective Properties (with inheritance):" << std::endl;
                print_paragraph_properties(effective_props_result.value(), "  ");
            }
        } else {
            print_error("Apply Heading 1 style", apply_result.error());
        }
    }
    
    // 3. Paragraph with direct formatting
    auto formatted_result = body.add_paragraph_safe("This paragraph has direct formatting applied.");
    if (formatted_result.ok()) {
        Paragraph* formatted_para = &formatted_result.value();
        
        // Apply formatting using StyleManager
        ParagraphStyleProperties props;
        props.alignment = Alignment::CENTER;
        props.space_before_pts = 12.0;
        props.space_after_pts = 6.0;
        
        auto apply_result = style_manager.apply_paragraph_properties_safe(*formatted_para, props);
        
        if (apply_result.ok()) {
            print_success("Applied direct formatting to paragraph");
            
            auto props_result = style_manager.read_paragraph_properties_safe(*formatted_para);
            if (props_result.ok()) {
                std::cout << "\n📝 Directly Formatted Paragraph:" << std::endl;
                print_paragraph_properties(props_result.value(), "  ");
            }
        }
    }
}

void demonstrate_character_style_reading(Body& body, StyleManager& style_manager)
{
    print_header("Character Style Reading Demonstration");
    
    // Create a paragraph with different runs
    auto para_result = body.add_paragraph_safe("");
    if (!para_result.ok()) {
        print_error("Create paragraph for character demo", para_result.error());
        return;
    }
    
    Paragraph* para = &para_result.value();
    
    // 1. Plain run
    Run& plain_run = para->add_run("Plain text, ");
    auto plain_props_result = style_manager.read_character_properties_safe(plain_run);
    if (plain_props_result.ok()) {
        std::cout << "\n🔤 Plain Run:" << std::endl;
        print_character_properties(plain_props_result.value(), "  ");
        if (!plain_props_result.value().font_name.has_value()) {
            std::cout << "  - No custom properties (uses default styling)" << std::endl;
        }
    }
    
    // 2. Run with direct formatting
    Run& formatted_run = para->add_run("bold and italic text, ");
    
    // Apply formatting using StyleManager
    CharacterStyleProperties char_props;
    char_props.font_name = "Arial";
    char_props.font_size_pts = 14.0;
    char_props.font_color_hex = "FF0000";
    char_props.formatting_flags = bold | italic;
    
    auto char_apply_result = style_manager.apply_character_properties_safe(formatted_run, char_props);
    
    if (char_apply_result.ok()) {
        print_success("Applied direct formatting to run");
        
        auto props_result = style_manager.read_character_properties_safe(formatted_run);
        if (props_result.ok()) {
            std::cout << "\n🔤 Formatted Run:" << std::endl;
            print_character_properties(props_result.value(), "  ");
        }
    }
    
    // 3. Run with applied style
    Run& styled_run = para->add_run("and code-styled text.");
    auto apply_result = styled_run.apply_style_safe(style_manager, "Code");
    if (apply_result.ok()) {
        print_success("Applied Code style to run");
        
        // Read effective properties
        auto effective_result = style_manager.get_effective_character_properties_safe(styled_run);
        if (effective_result.ok()) {
            std::cout << "\n🔤 Code-styled Run (effective properties):" << std::endl;
            print_character_properties(effective_result.value(), "  ");
        }
    } else {
        print_error("Apply Code style", apply_result.error());
    }
}

void demonstrate_table_style_reading(Body& body, StyleManager& style_manager)
{
    print_header("Table Style Reading Demonstration");
    
    // Create a table with formatting
    auto table_result = body.add_table_safe(3, 3);
    if (!table_result.ok()) {
        print_error("Create table", table_result.error());
        return;
    }
    
    Table* table = &table_result.value();
    
    // Add content to the table
    auto rows = table->rows();
    if (!rows.empty()) {
        auto& first_row = *rows.begin();
        auto cells = first_row.cells();
        if (!cells.empty()) {
            auto& first_cell = *cells.begin();
            // Use non-safe version since safe version doesn't exist for TableCell
            Paragraph para = first_cell.add_paragraph("Header 1");
            para.add_run("Sample Data");
        }
    }
    
    // Read properties before formatting
    auto initial_props_result = style_manager.read_table_properties_safe(*table);
    if (initial_props_result.ok()) {
        std::cout << "\n📊 Initial Table Properties:" << std::endl;
        print_table_properties(initial_props_result.value(), "  ");
        if (!initial_props_result.value().border_style.has_value()) {
            std::cout << "  - No custom properties (uses default styling)" << std::endl;
        }
    }
    
    // Apply some formatting using StyleManager
    TableStyleProperties table_props;
    table_props.table_width_pts = 500.0;
    table_props.table_alignment = "center";
    table_props.border_style = "single";
    
    auto table_apply_result = style_manager.apply_table_properties_safe(*table, table_props);
    
    if (table_apply_result.ok()) {
        print_success("Applied formatting to table");
        
        // Read properties after formatting
        auto formatted_props_result = style_manager.read_table_properties_safe(*table);
        if (formatted_props_result.ok()) {
            std::cout << "\n📊 Formatted Table Properties:" << std::endl;
            print_table_properties(formatted_props_result.value(), "  ");
        }
    } else {
        print_error("Apply table formatting", table_apply_result.error());
    }
}

void demonstrate_style_extraction(Body& body, StyleManager& style_manager)
{
    print_header("Style Extraction Demonstration");
    
    // Create a well-formatted paragraph
    auto para_result = body.add_paragraph_safe("This paragraph will be used to extract a custom style.");
    if (!para_result.ok()) {
        print_error("Create extraction paragraph", para_result.error());
        return;
    }
    
    Paragraph* para = &para_result.value();
    
    // Apply comprehensive formatting
    ParagraphStyleProperties extract_props;
    extract_props.alignment = Alignment::CENTER;
    extract_props.space_before_pts = 18.0;
    extract_props.space_after_pts = 12.0;
    
    auto extract_apply_result = style_manager.apply_paragraph_properties_safe(*para, extract_props);
    
    if (extract_apply_result.ok()) {
        print_success("Applied comprehensive formatting to paragraph");
        
        // Extract the style
        auto extract_result = style_manager.extract_style_from_element_safe(*para, "My Custom Title Style");
        if (extract_result.ok()) {
            Style* extracted_style = extract_result.value();
            print_success("Extracted style: " + extracted_style->name());
            
            std::cout << "\n🎨 Extracted Style Properties:" << std::endl;
            print_paragraph_properties(extracted_style->paragraph_properties(), "  ");
            
            // Now apply this extracted style to another paragraph
            auto new_para_result = body.add_paragraph_safe("This paragraph uses the extracted style.");
            if (new_para_result.ok()) {
                Paragraph* new_para = &new_para_result.value();
                
                auto apply_result = new_para->apply_style_safe(style_manager, "My Custom Title Style");
                if (apply_result.ok()) {
                    print_success("Applied extracted style to new paragraph");
                    
                    // Verify the properties were applied
                    auto verify_result = style_manager.get_effective_paragraph_properties_safe(*new_para);
                    if (verify_result.ok()) {
                        std::cout << "\n🎨 Verification - New Paragraph Properties:" << std::endl;
                        print_paragraph_properties(verify_result.value(), "  ");
                    }
                } else {
                    print_error("Apply extracted style", apply_result.error());
                }
            }
        } else {
            print_error("Extract style from paragraph", extract_result.error());
        }
    }
}

void demonstrate_style_comparison(StyleManager& style_manager)
{
    print_header("Style Comparison Demonstration");
    
    // Create two similar but different styles
    auto style1_result = style_manager.create_paragraph_style_safe("Style Comparison A");
    auto style2_result = style_manager.create_paragraph_style_safe("Style Comparison B");
    
    if (!style1_result.ok() || !style2_result.ok()) {
        print_error("Create comparison styles", style1_result.ok() ? style2_result.error() : style1_result.error());
        return;
    }
    
    Style* style1 = style1_result.value();
    Style* style2 = style2_result.value();
    
    // Set similar properties with some differences
    ParagraphStyleProperties props1;
    props1.alignment = Alignment::LEFT;
    props1.space_before_pts = 6.0;
    props1.space_after_pts = 6.0;
    
    ParagraphStyleProperties props2;
    props2.alignment = Alignment::CENTER; // Different alignment
    props2.space_before_pts = 6.0;        // Same
    props2.space_after_pts = 12.0;        // Different spacing
    
    auto set1_result = style1->set_paragraph_properties_safe(props1);
    auto set2_result = style2->set_paragraph_properties_safe(props2);
    
    if (set1_result.ok() && set2_result.ok()) {
        print_success("Created two styles with different properties");
        
        // Compare the styles
        auto compare_result = style_manager.compare_styles_safe("Style Comparison A", "Style Comparison B");
        if (compare_result.ok()) {
            std::cout << "\n🔍 Style Comparison Report:" << std::endl;
            std::cout << compare_result.value() << std::endl;
        } else {
            print_error("Compare styles", compare_result.error());
        }
        
        // Now make them identical and compare again
        auto modify_result = style2->set_paragraph_properties_safe(props1);
        if (modify_result.ok()) {
            print_success("Modified Style B to match Style A");
            
            auto identical_compare = style_manager.compare_styles_safe("Style Comparison A", "Style Comparison B");
            if (identical_compare.ok()) {
                std::cout << "\n🔍 Updated Comparison Report:" << std::endl;
                std::cout << identical_compare.value() << std::endl;
            }
        }
    }
}

void demonstrate_inheritance_resolution(Body& body, StyleManager& style_manager)
{
    print_header("Style Inheritance Resolution Demonstration");
    
    // Create a base style
    auto base_result = style_manager.create_paragraph_style_safe("Base Document Style");
    if (!base_result.ok()) {
        print_error("Create base style", base_result.error());
        return;
    }
    
    Style* base_style = base_result.value();
    
    // Set base properties
    ParagraphStyleProperties base_props;
    base_props.alignment = Alignment::LEFT;
    base_props.space_before_pts = 6.0;
    base_props.space_after_pts = 6.0;
    base_props.line_spacing = 1.15;
    
    auto set_base_result = base_style->set_paragraph_properties_safe(base_props);
    if (!set_base_result.ok()) {
        print_error("Set base style properties", set_base_result.error());
        return;
    }
    
    std::cout << "\n📐 Base Style Properties:" << std::endl;
    print_paragraph_properties(base_style->paragraph_properties(), "  ");
    
    // Create a derived style
    auto derived_result = style_manager.create_paragraph_style_safe("Custom Heading Style");
    if (!derived_result.ok()) {
        print_error("Create derived style", derived_result.error());
        return;
    }
    
    Style* derived_style = derived_result.value();
    
    // Set up inheritance
    auto inherit_result = derived_style->set_base_style_safe("Base Document Style");
    if (!inherit_result.ok()) {
        print_error("Set style inheritance", inherit_result.error());
        return;
    }
    
    // Override some properties in the derived style
    ParagraphStyleProperties derived_props;
    derived_props.alignment = Alignment::CENTER; // Override alignment
    derived_props.space_before_pts = 18.0;       // Override space before
    // Leave other properties to be inherited
    
    auto set_derived_result = derived_style->set_paragraph_properties_safe(derived_props);
    if (!set_derived_result.ok()) {
        print_error("Set derived style properties", set_derived_result.error());
        return;
    }
    
    std::cout << "\n📐 Derived Style Direct Properties:" << std::endl;
    print_paragraph_properties(derived_style->paragraph_properties(), "  ");
    
    // Demonstrate inheritance resolution by applying the derived style to a paragraph
    auto inheritance_para_result = body.add_paragraph_safe("Testing inheritance resolution");
    if (inheritance_para_result.ok()) {
        Paragraph* inheritance_para = &inheritance_para_result.value();
        auto apply_derived_result = style_manager.apply_paragraph_style_safe(*inheritance_para, "Custom Heading Style");
        
        if (apply_derived_result.ok()) {
            auto resolved_result = style_manager.get_effective_paragraph_properties_safe(*inheritance_para);
            if (resolved_result.ok()) {
                std::cout << "\n📐 Resolved Properties (with inheritance):" << std::endl;
                print_paragraph_properties(resolved_result.value(), "  ");
                
                const auto& resolved = resolved_result.value();
                
                std::cout << "\n🔗 Inheritance Analysis:" << std::endl;
                std::cout << "  - Alignment: " << (resolved.alignment.value() == Alignment::CENTER ? "Overridden in derived style" : "Inherited from base") << std::endl;
                std::cout << "  - Space Before: " << (resolved.space_before_pts.value() == 18.0 ? "Overridden in derived style" : "Inherited from base") << std::endl;
                std::cout << "  - Space After: " << (resolved.space_after_pts.value() == 6.0 ? "Inherited from base style" : "Overridden in derived style") << std::endl;
                std::cout << "  - Line Spacing: " << (resolved.line_spacing.has_value() ? "Inherited from base style" : "Not set") << std::endl;
            }
        }
    }
}

// ============================================================================
// Main Function
// ============================================================================

int main()
{
    std::cout << "🎨 DuckX Style Reading System Demonstration" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "This sample demonstrates comprehensive style reading capabilities" << std::endl;
    std::cout << "including property extraction, inheritance, and comparison." << std::endl;
    
    try {
        // Create a new document
        auto doc_result = Document::create_safe(test_utils::get_temp_path("sample22_style_reading_demo.docx"));
        if (!doc_result.ok()) {
            print_error("Create document", doc_result.error());
            return 1;
        }
        
        Document doc = std::move(doc_result.value());
        
        Body& body = doc.body();
        
        // Create and configure style manager
        StyleManager style_manager;
        
        // Load built-in styles
        auto load_result = style_manager.load_all_built_in_styles_safe();
        if (!load_result.ok()) {
            print_error("Load built-in styles", load_result.error());
            return 1;
        }
        
        print_success("Loaded built-in styles for demonstration");
        
        // Run all demonstrations
        demonstrate_paragraph_style_reading(body, style_manager);
        demonstrate_character_style_reading(body, style_manager);
        demonstrate_table_style_reading(body, style_manager);
        demonstrate_style_extraction(body, style_manager);
        demonstrate_style_comparison(style_manager);
        demonstrate_inheritance_resolution(body, style_manager);
        
        // Save the document with all the examples
        auto save_result = doc.save_safe();
        if (save_result.ok()) {
            print_success("Saved demonstration document");
        } else {
            print_error("Save document", save_result.error());
        }
        
        print_header("Style Reading System Summary");
        std::cout << "✨ The style reading system provides:" << std::endl;
        std::cout << "   • Direct property reading from elements" << std::endl;
        std::cout << "   • Effective property resolution with inheritance" << std::endl;
        std::cout << "   • Style extraction from formatted elements" << std::endl;
        std::cout << "   • Style comparison and analysis" << std::endl;
        std::cout << "   • Comprehensive inheritance chain resolution" << std::endl;
        std::cout << "   • Full Result<T> error handling support" << std::endl;
        
        std::cout << "\n🎯 Total styles in manager: " << style_manager.style_count() << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "❌ Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}