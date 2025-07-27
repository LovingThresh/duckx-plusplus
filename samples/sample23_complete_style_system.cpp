/*!
 * @file sample23_complete_style_system.cpp
 * @brief Complete demonstration of the integrated style application and reading system
 * 
 * This sample showcases the full cycle of style management: creating styles,
 * applying them to elements, reading properties back, and using the modern
 * Result<T> error handling throughout.
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
// Utility Functions
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

void print_paragraph_summary(const ParagraphStyleProperties& props)
{
    std::cout << "  📝 Paragraph Properties: ";
    
    std::vector<std::string> applied;
    
    if (props.alignment.has_value()) {
        std::string align_str;
        switch (props.alignment.value()) {
            case Alignment::LEFT: align_str = "left"; break;
            case Alignment::CENTER: align_str = "center"; break;
            case Alignment::RIGHT: align_str = "right"; break;
            case Alignment::BOTH: align_str = "justify"; break;
        }
        applied.push_back("alignment=" + align_str);
    }
    
    if (props.space_before_pts.has_value()) {
        applied.push_back("before=" + std::to_string(props.space_before_pts.value()) + "pts");
    }
    
    if (props.space_after_pts.has_value()) {
        applied.push_back("after=" + std::to_string(props.space_after_pts.value()) + "pts");
    }
    
    if (props.line_spacing.has_value()) {
        applied.push_back("line=" + std::to_string(props.line_spacing.value()));
    }
    
    if (applied.empty()) {
        std::cout << "none" << std::endl;
    } else {
        for (size_t i = 0; i < applied.size(); ++i) {
            std::cout << applied[i];
            if (i < applied.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
}

void print_character_summary(const CharacterStyleProperties& props)
{
    std::cout << "  🔤 Character Properties: ";
    
    std::vector<std::string> applied;
    
    if (props.font_name.has_value()) {
        applied.push_back("font=" + props.font_name.value());
    }
    
    if (props.font_size_pts.has_value()) {
        applied.push_back("size=" + std::to_string(props.font_size_pts.value()) + "pts");
    }
    
    if (props.font_color_hex.has_value()) {
        applied.push_back("color=#" + props.font_color_hex.value());
    }
    
    if (props.formatting_flags.has_value()) {
        std::vector<std::string> formats;
        formatting_flag flags = props.formatting_flags.value();
        if (flags & bold) formats.push_back("bold");
        if (flags & italic) formats.push_back("italic");
        if (flags & underline) formats.push_back("underline");
        if (flags & strikethrough) formats.push_back("strike");
        if (flags & smallcaps) formats.push_back("smallcaps");
        
        if (!formats.empty()) {
            applied.push_back("format=" + formats[0]);
            for (size_t i = 1; i < formats.size(); ++i) {
                applied.back() += "+" + formats[i];
            }
        }
    }
    
    if (applied.empty()) {
        std::cout << "none" << std::endl;
    } else {
        for (size_t i = 0; i < applied.size(); ++i) {
            std::cout << applied[i];
            if (i < applied.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
}

// ============================================================================
// Style Application and Reading Demonstration
// ============================================================================

void demonstrate_style_creation_and_application(Body& body, StyleManager& style_manager)
{
    print_header("Style Creation and Application");
    
    // Create a comprehensive document title style
    auto title_style_result = style_manager.create_mixed_style_safe("Document Title");
    if (!title_style_result.ok()) {
        print_error("Create title style", title_style_result.error());
        return;
    }
    
    Style* title_style = title_style_result.value();
    
    // Configure paragraph properties
    ParagraphStyleProperties para_props;
    para_props.alignment = Alignment::CENTER;
    para_props.space_before_pts = 24.0;
    para_props.space_after_pts = 18.0;
    para_props.line_spacing = 1.2;
    
    auto set_para_result = title_style->set_paragraph_properties_safe(para_props);
    if (!set_para_result.ok()) {
        print_error("Set paragraph properties", set_para_result.error());
        return;
    }
    
    // Configure character properties
    CharacterStyleProperties char_props;
    char_props.font_name = "Calibri";
    char_props.font_size_pts = 18.0;
    char_props.font_color_hex = "2F4F4F";
    char_props.formatting_flags = bold;
    
    auto set_char_result = title_style->set_character_properties_safe(char_props);
    if (!set_char_result.ok()) {
        print_error("Set character properties", set_char_result.error());
        return;
    }
    
    print_success("Created 'Document Title' style with comprehensive formatting");
    
    // Apply the style to a paragraph
    auto para_result = body.add_paragraph_safe("Complete Style System Demonstration");
    if (!para_result.ok()) {
        print_error("Create title paragraph", para_result.error());
        return;
    }
    
    Paragraph* title_para = &para_result.value();
    
    auto apply_result = style_manager.apply_paragraph_style_safe(*title_para, "Document Title");
    if (!apply_result.ok()) {
        print_error("Apply title style", apply_result.error());
        return;
    }
    
    print_success("Applied 'Document Title' style to paragraph");
    
    // Read back and verify the applied properties
    auto read_para_result = style_manager.read_paragraph_properties_safe(*title_para);
    if (read_para_result.ok()) {
        print_paragraph_summary(read_para_result.value());
    }
    
    // For character properties, we need to work with runs
    auto& first_run = *title_para->runs().begin();
    auto char_apply_result = style_manager.apply_character_style_safe(first_run, "Document Title");
    if (char_apply_result.ok()) {
        print_success("Applied character formatting to title text");
        
        auto read_char_result = style_manager.read_character_properties_safe(first_run);
        if (read_char_result.ok()) {
            print_character_summary(read_char_result.value());
        }
    }
}

void demonstrate_property_extraction_and_reuse(Body& body, StyleManager& style_manager)
{
    print_header("Property Extraction and Style Reuse");
    
    // Create a formatted paragraph with direct formatting
    auto para_result = body.add_paragraph_safe("This paragraph has custom formatting that we'll extract.");
    if (!para_result.ok()) {
        print_error("Create source paragraph", para_result.error());
        return;
    }
    
    Paragraph* source_para = &para_result.value();
    
    // Apply direct formatting using StyleManager
    ParagraphStyleProperties direct_props;
    direct_props.alignment = Alignment::RIGHT;
    direct_props.space_before_pts = 15.0;
    direct_props.space_after_pts = 10.0;
    
    auto direct_apply_result = style_manager.apply_paragraph_properties_safe(*source_para, direct_props);
    
    if (direct_apply_result.ok()) {
        print_success("Applied direct formatting to source paragraph");
        
        // Read the properties
        auto read_result = style_manager.read_paragraph_properties_safe(*source_para);
        if (read_result.ok()) {
            std::cout << "  📋 Original formatting:" << std::endl;
            print_paragraph_summary(read_result.value());
        }
        
        // Extract the formatting as a new style
        auto extract_result = style_manager.extract_style_from_element_safe(*source_para, "Extracted Right Align");
        if (extract_result.ok()) {
            print_success("Extracted formatting as 'Extracted Right Align' style");
            
            // Apply the extracted style to a new paragraph
            auto new_para_result = body.add_paragraph_safe("This paragraph uses the extracted style.");
            if (new_para_result.ok()) {
                Paragraph* new_para = &new_para_result.value();
                
                auto reapply_result = style_manager.apply_paragraph_style_safe(*new_para, "Extracted Right Align");
                if (reapply_result.ok()) {
                    print_success("Applied extracted style to new paragraph");
                    
                    // Verify the properties were transferred
                    auto verify_result = style_manager.read_paragraph_properties_safe(*new_para);
                    if (verify_result.ok()) {
                        std::cout << "  ✨ Reapplied formatting:" << std::endl;
                        print_paragraph_summary(verify_result.value());
                    }
                }
            }
        } else {
            print_error("Extract style", extract_result.error());
        }
    }
}

void demonstrate_table_styling(Body& body, StyleManager& style_manager)
{
    print_header("Table Style Management");
    
    // Create a custom table style
    auto table_style_result = style_manager.create_table_style_safe("Professional Table");
    if (!table_style_result.ok()) {
        print_error("Create table style", table_style_result.error());
        return;
    }
    
    Style* table_style = table_style_result.value();
    
    // Configure table properties
    TableStyleProperties table_props;
    table_props.table_width_pts = 600.0;
    table_props.table_alignment = "center";
    table_props.border_style = "single";
    table_props.border_width_pts = 1.5;
    table_props.border_color_hex = "333333";
    table_props.cell_padding_pts = 8.0;
    
    auto set_table_result = table_style->set_table_properties_safe(table_props);
    if (!set_table_result.ok()) {
        print_error("Set table properties", set_table_result.error());
        return;
    }
    
    print_success("Created 'Professional Table' style");
    
    // Create a table and apply the style
    auto table_result = body.add_table_safe(3, 3);
    if (!table_result.ok()) {
        print_error("Create table", table_result.error());
        return;
    }
    
    Table* table = &table_result.value();
    
    // Add some content
    auto rows = table->rows();
    if (!rows.empty()) {
        auto& first_row = *rows.begin();
        auto cells = first_row.cells();
        if (!cells.empty()) {
            auto& first_cell = *cells.begin();
            // Use non-safe version since safe version doesn't exist for TableCell
            Paragraph cell_para = first_cell.add_paragraph("Header 1");
            cell_para.add_run("Sample Data");
        }
    }
    
    // Apply the table style
    auto apply_table_result = style_manager.apply_table_style_safe(*table, "Professional Table");
    if (apply_table_result.ok()) {
        print_success("Applied 'Professional Table' style");
        
        // Read back the properties
        auto read_table_result = style_manager.read_table_properties_safe(*table);
        if (read_table_result.ok()) {
            const auto& props = read_table_result.value();
            std::cout << "  📊 Table Properties: ";
            
            std::vector<std::string> applied;
            if (props.table_width_pts.has_value()) {
                applied.push_back("width=" + std::to_string(props.table_width_pts.value()) + "pts");
            }
            if (props.table_alignment.has_value()) {
                applied.push_back("align=" + props.table_alignment.value());
            }
            if (props.border_style.has_value()) {
                applied.push_back("border=" + props.border_style.value());
            }
            if (props.cell_padding_pts.has_value()) {
                applied.push_back("padding=" + std::to_string(props.cell_padding_pts.value()) + "pts");
            }
            
            for (size_t i = 0; i < applied.size(); ++i) {
                std::cout << applied[i];
                if (i < applied.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
    } else {
        print_error("Apply table style", apply_table_result.error());
    }
}

void demonstrate_style_inheritance_and_comparison(Body& body, StyleManager& style_manager)
{
    print_header("Style Inheritance and Comparison");
    
    // Create a base style
    auto base_result = style_manager.create_paragraph_style_safe("Base Body Text");
    if (!base_result.ok()) {
        print_error("Create base style", base_result.error());
        return;
    }
    
    Style* base_style = base_result.value();
    
    ParagraphStyleProperties base_props;
    base_props.alignment = Alignment::LEFT;
    base_props.space_before_pts = 6.0;
    base_props.space_after_pts = 6.0;
    base_props.line_spacing = 1.15;
    
    auto set_base_result = base_style->set_paragraph_properties_safe(base_props);
    if (!set_base_result.ok()) {
        print_error("Set base properties", set_base_result.error());
        return;
    }
    
    print_success("Created 'Base Body Text' style");
    
    // Create a derived style
    auto derived_result = style_manager.create_paragraph_style_safe("Emphasized Body Text");
    if (!derived_result.ok()) {
        print_error("Create derived style", derived_result.error());
        return;
    }
    
    Style* derived_style = derived_result.value();
    
    // Set inheritance
    auto inherit_result = derived_style->set_base_style_safe("Base Body Text");
    if (!inherit_result.ok()) {
        print_error("Set inheritance", inherit_result.error());
        return;
    }
    
    // Override some properties
    ParagraphStyleProperties derived_props;
    derived_props.alignment = Alignment::CENTER;  // Override alignment
    derived_props.space_before_pts = 12.0;        // Override space before
    // Leave other properties to inherit
    
    auto set_derived_result = derived_style->set_paragraph_properties_safe(derived_props);
    if (!set_derived_result.ok()) {
        print_error("Set derived properties", set_derived_result.error());
        return;
    }
    
    print_success("Created 'Emphasized Body Text' style with inheritance");
    
    // Test inheritance resolution by applying style to a paragraph
    auto test_para_result = body.add_paragraph_safe("Testing inheritance resolution");
    if (test_para_result.ok()) {
        Paragraph* test_para = &test_para_result.value();
        auto apply_result = style_manager.apply_paragraph_style_safe(*test_para, "Emphasized Body Text");
        
        if (apply_result.ok()) {
            auto resolved_result = style_manager.get_effective_paragraph_properties_safe(*test_para);
            if (resolved_result.ok()) {
                std::cout << "\n🔗 Inheritance Resolution Results:" << std::endl;
                print_paragraph_summary(resolved_result.value());
                
                const auto& resolved = resolved_result.value();
                std::cout << "\n  📋 Inheritance Analysis:" << std::endl;
                std::cout << "    • Alignment: " << (resolved.alignment.value() == Alignment::CENTER ? "Overridden" : "Inherited") << std::endl;
                std::cout << "    • Space Before: " << (resolved.space_before_pts.value() == 12.0 ? "Overridden" : "Inherited") << std::endl;
                std::cout << "    • Space After: " << (resolved.space_after_pts.value() == 6.0 ? "Inherited" : "Overridden") << std::endl;
                std::cout << "    • Line Spacing: " << (resolved.line_spacing.has_value() ? "Inherited" : "Not set") << std::endl;
            }
        }
    }
    
    // Compare the two styles
    auto compare_result = style_manager.compare_styles_safe("Base Body Text", "Emphasized Body Text");
    if (compare_result.ok()) {
        std::cout << "\n🔍 Style Comparison Report:" << std::endl;
        std::cout << compare_result.value() << std::endl;
    }
}

void demonstrate_comprehensive_workflow(Body& body, StyleManager& style_manager)
{
    print_header("Comprehensive Style Workflow");
    
    std::cout << "📚 Creating a styled document section..." << std::endl;
    
    // Section heading
    auto heading_result = body.add_paragraph_safe("Advanced Features");
    if (heading_result.ok()) {
        Paragraph* heading = &heading_result.value();
        
        auto apply_heading_result = style_manager.apply_paragraph_style_safe(*heading, "Heading 1");
        if (apply_heading_result.ok()) {
            print_success("Applied Heading 1 style to section heading");
        }
    }
    
    // Body paragraph with mixed formatting
    auto body_para_result = body.add_paragraph_safe("");
    if (body_para_result.ok()) {
        Paragraph* body_para = &body_para_result.value();
        
        // Apply custom paragraph style if available
        auto custom_apply_result = style_manager.apply_paragraph_style_safe(*body_para, "Base Body Text");
        if (custom_apply_result.ok()) {
            print_success("Applied custom paragraph style to body text");
        }
        
        // Add runs with different character styles
        Run& normal_run = body_para->add_run("This document demonstrates ");
        Run& emphasized_run = body_para->add_run("advanced style management");
        body_para->add_run(" capabilities including ");
        Run& code_run = body_para->add_run("property extraction");
        body_para->add_run(" and ");
        Run& bold_run = body_para->add_run("inheritance resolution");
        body_para->add_run(".");
        
        // Apply character styles
        auto code_apply_result = style_manager.apply_character_style_safe(code_run, "Code");
        if (code_apply_result.ok()) {
            print_success("Applied Code style to technical term");
        }
        
        // Apply direct formatting to other runs using StyleManager
        CharacterStyleProperties bold_props;
        bold_props.formatting_flags = bold;
        auto bold_result = style_manager.apply_character_properties_safe(bold_run, bold_props);
        
        CharacterStyleProperties emphasized_props;
        emphasized_props.formatting_flags = italic;
        emphasized_props.font_color_hex = "0066CC";
        auto emphasized_result = style_manager.apply_character_properties_safe(emphasized_run, emphasized_props);
        
        if (bold_result.ok() && emphasized_result.ok()) {
            print_success("Applied direct formatting to emphasized text");
        }
    }
    
    // Summary statistics
    std::cout << "\n📊 Style Management Summary:" << std::endl;
    std::cout << "  • Total styles in manager: " << style_manager.style_count() << std::endl;
    
    auto all_names = style_manager.get_all_style_names();
    auto paragraph_names = style_manager.get_style_names_by_type(StyleType::PARAGRAPH);
    auto character_names = style_manager.get_style_names_by_type(StyleType::CHARACTER);
    auto table_names = style_manager.get_style_names_by_type(StyleType::TABLE);
    auto mixed_names = style_manager.get_style_names_by_type(StyleType::MIXED);
    
    std::cout << "  • Paragraph styles: " << paragraph_names.size() << std::endl;
    std::cout << "  • Character styles: " << character_names.size() << std::endl;
    std::cout << "  • Table styles: " << table_names.size() << std::endl;
    std::cout << "  • Mixed styles: " << mixed_names.size() << std::endl;
}

// ============================================================================
// Main Function
// ============================================================================

int main()
{
    std::cout << "🎨 Complete DuckX Style System Demonstration" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Showcasing integrated style application and reading capabilities" << std::endl;
    
    try {
        // Create document and setup
        auto doc_result = Document::create_safe(test_utils::get_temp_path("sample23_complete_style_demo.docx"));
        if (!doc_result.ok()) {
            print_error("Create document", doc_result.error());
            return 1;
        }
        
        Document doc = std::move(doc_result.value());
        
        Body& body = doc.body();
        
        // Initialize style manager
        StyleManager style_manager;
        
        auto load_result = style_manager.load_all_built_in_styles_safe();
        if (!load_result.ok()) {
            print_error("Load built-in styles", load_result.error());
            return 1;
        }
        
        print_success("Initialized style manager with built-in styles");
        
        // Run all demonstrations
        demonstrate_style_creation_and_application(body, style_manager);
        demonstrate_property_extraction_and_reuse(body, style_manager);
        demonstrate_table_styling(body, style_manager);
        demonstrate_style_inheritance_and_comparison(body, style_manager);
        demonstrate_comprehensive_workflow(body, style_manager);
        
        // Save the demonstration document
        auto save_result = doc.save_safe();
        if (save_result.ok()) {
            print_success("Saved demonstration document");
        } else {
            print_error("Save document", save_result.error());
        }
        
        print_header("Style System Features Summary");
        std::cout << "✨ The complete style system provides:" << std::endl;
        std::cout << "   📝 Style creation and management" << std::endl;
        std::cout << "   🎯 Property-based style application" << std::endl;
        std::cout << "   📖 Comprehensive style reading" << std::endl;
        std::cout << "   🔗 Style inheritance and resolution" << std::endl;
        std::cout << "   🔍 Style comparison and analysis" << std::endl;
        std::cout << "   ♻️  Style extraction and reuse" << std::endl;
        std::cout << "   🛡️  Full Result<T> error handling" << std::endl;
        std::cout << "   📊 Built-in and custom style support" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "❌ Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}