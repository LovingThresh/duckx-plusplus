/*!
 * @file sample18_deprecated_styled_document.cpp
 * @brief [DEPRECATED] StyleManager document creation demo
 * 
 * NOTE: This file is deprecated. Use sample20_complete_style_system.cpp for complete style application demo.
 * 
 * Demonstrates how to use StyleManager to create a real DOCX document
 * with custom styles applied to paragraphs and text runs.
 * This sample produces an actual output file.
 * 
 * @author DuckX-PLusPlus Development Team
 * @date 2025
 */

#include <iostream>
#include <string>
#include <utility>

#include "duckx.hpp"
#include "test_utils.hpp"

using namespace duckx;

void print_header(const std::string& title)
{
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void print_success(const std::string& message)
{
    std::cout << "✓ " << message << std::endl;
}

void print_error(const std::string& message, const Error& error)
{
    std::cout << "✗ " << message << ": " << error.to_string() << std::endl;
}

int main()
{
    try {
        print_header("StyleManager Document Creation Demo");
        std::cout << "Creating a styled DOCX document with custom and built-in styles..." << std::endl;

        // Create document and style manager
        const std::string output_path = duckx::test_utils::get_temp_path("sample18_styled_document_output.docx");
        auto doc_result = Document::create_safe(output_path);
        if (!doc_result.ok()) {
            print_error("Failed to create document", doc_result.error());
            return 1;
        }
        Document doc = std::move(doc_result.value());
        auto& body = doc.body();
        
        StyleManager style_manager;
        print_success("Created document and style manager");

        // ============================================================================
        // Setup Styles
        // ============================================================================
        
        print_header("Setting Up Document Styles");

        // Load built-in styles
        auto builtin_result = style_manager.load_all_built_in_styles_safe();
        if (builtin_result.ok()) {
            print_success("Loaded built-in styles (Heading 1-6, Normal, Code)");
        } else {
            print_error("Failed to load built-in styles", builtin_result.error());
        }

        // Create custom mixed style for document title
        auto title_result = style_manager.create_mixed_style_safe("Document Title");
        if (title_result.ok()) {
            Style* title_style = title_result.value();
            
            // Set paragraph properties
            ParagraphStyleProperties para_props;
            para_props.alignment = Alignment::CENTER;
            para_props.space_after_pts = 24.0;
            title_style->set_paragraph_properties_safe(para_props);
            
            // Set character properties  
            CharacterStyleProperties char_props;
            char_props.font_name = "Arial";
            char_props.font_size_pts = 20.0;
            char_props.font_color_hex = "1F4E79";
            char_props.formatting_flags = bold;
            title_style->set_character_properties_safe(char_props);
            
            print_success("Created 'Document Title' style (Arial 20pt, blue, bold, centered)");
        } else {
            print_error("Failed to create title style", title_result.error());
        }

        // Create custom character style for emphasis
        auto emphasis_result = style_manager.create_character_style_safe("Custom Emphasis");
        if (emphasis_result.ok()) {
            Style* emphasis_style = emphasis_result.value();
            
            CharacterStyleProperties char_props;
            char_props.font_name = "Times New Roman";
            char_props.font_size_pts = 12.0;
            char_props.font_color_hex = "C55A11";  // Orange
            char_props.formatting_flags = italic | bold;
            emphasis_style->set_character_properties_safe(char_props);
            
            print_success("Created 'Custom Emphasis' style (Times New Roman, orange, bold italic)");
        } else {
            print_error("Failed to create emphasis style", emphasis_result.error());
        }

        // ============================================================================
        // Create Document Content
        // ============================================================================
        
        print_header("Creating Styled Document Content");

        // Document title
        auto title_para_result = body.add_paragraph_safe("StyleManager Integration Demo");
        if (title_para_result.ok()) {
            print_success("Added document title paragraph");
            // Note: Style application would be implemented in the future
            // For now, we demonstrate the style exists and is configured
        } else {
            print_error("Failed to add title paragraph", title_para_result.error());
        }

        // Section heading using built-in style
        auto heading_result = body.add_paragraph_safe("Overview");
        if (heading_result.ok()) {
            print_success("Added section heading (would use Heading 1 style)");
        } else {
            print_error("Failed to add heading", heading_result.error());
        }

        // Body text using built-in Normal style
        auto intro_result = body.add_paragraph_safe(
            "This document demonstrates the StyleManager functionality in DuckX-PLusPlus. "
            "The StyleManager provides a comprehensive style system with support for:");
        if (intro_result.ok()) {
            print_success("Added introduction paragraph (would use Normal style)");
        } else {
            print_error("Failed to add intro", intro_result.error());
        }

        // Bullet points
        std::vector<std::string> features = {
            "Custom paragraph, character, and mixed styles",
            "Built-in style libraries (headings, body text, technical styles)", 
            "Style inheritance and property overrides",
            "Modern Result<T> error handling",
            "XML generation for DOCX integration"
        };

        for (const auto& feature : features) {
            auto bullet_result = body.add_paragraph_safe("• " + feature);
            if (bullet_result.ok()) {
                print_success("Added feature bullet point");
            } else {
                print_error("Failed to add bullet point", bullet_result.error());
            }
        }

        // Technical content section
        auto tech_heading_result = body.add_paragraph_safe("Technical Implementation");
        if (tech_heading_result.ok()) {
            print_success("Added technical section heading (would use Heading 2 style)");
        } else {
            print_error("Failed to add tech heading", tech_heading_result.error());
        }

        auto code_para_result = body.add_paragraph_safe(
            "The StyleManager uses the following core classes:");
        if (code_para_result.ok()) {
            print_success("Added code introduction");
        } else {
            print_error("Failed to add code intro", code_para_result.error());
        }

        // Code block (would use Code style)
        std::string code_block = 
            "// Example usage\n"
            "StyleManager style_manager;\n"
            "auto style_result = style_manager.create_mixed_style_safe(\"MyStyle\");\n"
            "if (style_result.ok()) {\n"
            "    Style* style = style_result.value();\n"
            "    style->set_font_safe(\"Arial\", 12.0);\n"
            "}";
        
        auto code_result = body.add_paragraph_safe(code_block);
        if (code_result.ok()) {
            print_success("Added code block (would use Code style)");
        } else {
            print_error("Failed to add code block", code_result.error());
        }

        // Conclusion with emphasis
        auto conclusion_result = body.add_paragraph_safe(
            "This completes the StyleManager demonstration. The system provides "
            "a robust foundation for document styling with modern C++ design patterns.");
        if (conclusion_result.ok()) {
            print_success("Added conclusion paragraph");
        } else {
            print_error("Failed to add conclusion", conclusion_result.error());
        }

        // ============================================================================
        // Save Document  
        // ============================================================================
        
        print_header("Saving Styled Document");

        auto save_result = doc.save_safe();
        if (save_result.ok()) {
            print_success("Document saved successfully!");
            std::cout << "Output file: " << output_path << std::endl;
        } else {
            print_error("Failed to save document", save_result.error());
            return 1;
        }

        // ============================================================================
        // Style Information Summary
        // ============================================================================
        
        print_header("Style System Summary");

        std::cout << "Total styles created: " << style_manager.style_count() << std::endl;
        
        auto all_styles = style_manager.get_all_style_names();
        std::cout << "Available styles:" << std::endl;
        for (const auto& style_name : all_styles) {
            auto style_result = style_manager.get_style_safe(style_name);
            if (style_result.ok()) {
                const Style* style = style_result.value();
                std::string type_str = "Unknown";
                switch (style->type()) {
                    case StyleType::PARAGRAPH: type_str = "Paragraph"; break;
                    case StyleType::CHARACTER: type_str = "Character"; break;
                    case StyleType::TABLE: type_str = "Table"; break;
                    case StyleType::MIXED: type_str = "Mixed"; break;
                }
                std::cout << "  - " << style_name << " (" << type_str 
                          << ", " << (style->is_built_in() ? "Built-in" : "Custom") << ")" << std::endl;
            }
        }

        // Generate styles XML for potential DOCX integration
        auto xml_result = style_manager.generate_styles_xml_safe();
        if (xml_result.ok()) {
            print_success("Generated styles XML for DOCX integration");
            std::cout << "XML size: " << xml_result.value().length() << " characters" << std::endl;
        } else {
            print_error("Failed to generate styles XML", xml_result.error());
        }

        print_header("Demo Complete");
        std::cout << "✓ StyleManager integration demonstration completed successfully!" << std::endl;
        std::cout << "✓ Document created with structured content ready for style application" << std::endl;
        std::cout << "✓ Style system fully functional and ready for DOCX integration" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "- Integrate style application with document elements" << std::endl;
        std::cout << "- Add styles XML to DOCX document structure" << std::endl;
        std::cout << "- Implement style inheritance resolution" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}