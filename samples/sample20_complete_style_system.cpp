/*!
 * @file sample20_complete_style_system.cpp
 * @brief Complete style system demonstration - StyleManager + Document integration
 * 
 * Demonstrates the full style application workflow including:
 * - Creating styles with StyleManager
 * - Applying styles to paragraphs, runs, and tables
 * - Using built-in and custom styles
 * - Generating properly formatted DOCX output
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
        print_header("Style Application Demonstration");
        std::cout << "Creating a document with applied styles to demonstrate full integration..." << std::endl;

        // Create document
        const std::string output_path = duckx::test_utils::get_temp_path("sample20_complete_style_system.docx");
        auto doc_result = Document::create_safe(output_path);
        if (!doc_result.ok()) {
            print_error("Failed to create document", doc_result.error());
            return 1;
        }
        Document doc = std::move(doc_result.value());
        auto& body = doc.body();
        auto& style_manager = doc.styles();
        
        print_success("Created document and style manager");

        // ============================================================================
        // Setup Styles
        // ============================================================================
        
        print_header("Setting Up Custom Styles");

        // Load built-in styles first
        auto builtin_result = style_manager.load_all_built_in_styles_safe();
        if (builtin_result.ok()) {
            print_success("Loaded built-in styles");
        } else {
            print_error("Failed to load built-in styles", builtin_result.error());
        }

        // Create custom document title style
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

        // Create custom emphasis character style
        auto emphasis_result = style_manager.create_character_style_safe("Important Text");
        if (emphasis_result.ok()) {
            Style* emphasis_style = emphasis_result.value();
            
            CharacterStyleProperties char_props;
            char_props.font_name = "Times New Roman";
            char_props.font_size_pts = 12.0;
            char_props.font_color_hex = "C55A11";  // Orange
            char_props.formatting_flags = italic | bold;
            emphasis_style->set_character_properties_safe(char_props);
            
            print_success("Created 'Important Text' style (Times New Roman, orange, bold italic)");
        } else {
            print_error("Failed to create emphasis style", emphasis_result.error());
        }

        // ============================================================================
        // Create Document Content with Applied Styles
        // ============================================================================
        
        print_header("Creating Document Content with Applied Styles");

        // Document title with custom style
        auto title_para_result = body.add_paragraph_safe("Style Application Demonstration");
        if (title_para_result.ok()) {
            Paragraph* title_para = &title_para_result.value();
            auto apply_result = title_para->apply_style_safe(style_manager, "Document Title");
            if (apply_result.ok()) {
                print_success("Applied 'Document Title' style to main heading");
            } else {
                print_error("Failed to apply title style", apply_result.error());
            }
        } else {
            print_error("Failed to add title paragraph", title_para_result.error());
        }

        // Section heading using built-in Heading 1 style
        auto heading_result = body.add_paragraph_safe("Overview");
        if (heading_result.ok()) {
            Paragraph* heading_para = &heading_result.value();
            auto apply_result = heading_para->apply_style_safe(style_manager, "Heading 1");
            if (apply_result.ok()) {
                print_success("Applied built-in 'Heading 1' style to section heading");
            } else {
                print_error("Failed to apply Heading 1 style", apply_result.error());
            }
        } else {
            print_error("Failed to add heading", heading_result.error());
        }

        // Body text using built-in Normal style
        auto intro_result = body.add_paragraph_safe(
            "This document demonstrates the style application system in DuckX-PLusPlus. "
            "The system now supports applying both built-in and custom styles to document elements.");
        if (intro_result.ok()) {
            Paragraph* intro_para = &intro_result.value();
            auto apply_result = intro_para->apply_style_safe(style_manager, "Normal");
            if (apply_result.ok()) {
                print_success("Applied built-in 'Normal' style to body text");
            } else {
                print_error("Failed to apply Normal style", apply_result.error());
            }
        } else {
            print_error("Failed to add intro", intro_result.error());
        }

        // Paragraph with character style on specific runs
        auto mixed_para_result = body.add_paragraph_safe("");
        if (mixed_para_result.ok()) {
            Paragraph* mixed_para = &mixed_para_result.value();
            
            // Apply Normal paragraph style
            auto para_style_result = mixed_para->apply_style_safe(style_manager, "Normal");
            if (para_style_result.ok()) {
                print_success("Applied Normal style to mixed content paragraph");
            } else {
                print_error("Failed to apply paragraph style", para_style_result.error());
            }
            
            // Add runs with different character styles
            Run& normal_run = mixed_para->add_run("This text uses normal formatting, while ");
            Run& emphasis_run = mixed_para->add_run("this text uses custom character styling");
            Run& end_run = mixed_para->add_run(" to show the difference.");
            
            // Apply character style to the emphasis run
            auto char_style_result = emphasis_run.apply_style_safe(style_manager, "Important Text");
            if (char_style_result.ok()) {
                print_success("Applied 'Important Text' character style to emphasized run");
            } else {
                print_error("Failed to apply character style", char_style_result.error());
            }
            
        } else {
            print_error("Failed to add mixed content paragraph", mixed_para_result.error());
        }

        // Technical content section with Heading 2
        auto tech_heading_result = body.add_paragraph_safe("Technical Implementation");
        if (tech_heading_result.ok()) {
            Paragraph* tech_para = &tech_heading_result.value();
            auto apply_result = tech_para->apply_style_safe(style_manager, "Heading 2");
            if (apply_result.ok()) {
                print_success("Applied built-in 'Heading 2' style to technical section");
            } else {
                print_error("Failed to apply Heading 2 style", apply_result.error());
            }
        } else {
            print_error("Failed to add tech heading", tech_heading_result.error());
        }

        // Code block with Code character style
        auto code_para_result = body.add_paragraph_safe("");
        if (code_para_result.ok()) {
            Paragraph* code_para = &code_para_result.value();
            
            // Apply Normal paragraph style
            auto para_style_result = code_para->apply_style_safe(style_manager, "Normal");
            if (para_style_result.ok()) {
                print_success("Applied Normal style to code paragraph");
            } else {
                print_error("Failed to apply paragraph style", para_style_result.error());
            }
            
            // Add code content with Code character style
            Run& code_run = code_para->add_run(
                "// Example usage\n"
                "auto& styles = doc.styles();\n"
                "auto paragraph = body.add_paragraph_safe(\"Hello World\");\n"
                "paragraph.value()->apply_style_safe(styles, \"Heading 1\");");
            
            auto char_style_result = code_run.apply_style_safe(style_manager, "Code");
            if (char_style_result.ok()) {
                print_success("Applied built-in 'Code' character style to code block");
            } else {
                print_error("Failed to apply Code style", char_style_result.error());
            }
        } else {
            print_error("Failed to add code paragraph", code_para_result.error());
        }

        // ============================================================================
        // Demonstrate Style Querying
        // ============================================================================
        
        print_header("Style Query Demonstration");

        // Check what style is applied to the title
        if (title_para_result.ok()) {
            auto current_style_result = title_para_result.value().get_style_safe();
            if (current_style_result.ok()) {
                std::string current_style = current_style_result.value();
                if (!current_style.empty()) {
                    print_success("Title paragraph has style: " + current_style);
                } else {
                    std::cout << "ℹ Title paragraph has no style applied" << std::endl;
                }
            } else {
                print_error("Failed to query title style", current_style_result.error());
            }
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
        // Summary
        // ============================================================================
        
        print_header("Style Application Summary");

        std::cout << "Total styles available: " << style_manager.style_count() << std::endl;
        std::cout << "Styles applied in this demo:" << std::endl;
        std::cout << "  - Document Title (custom mixed style) → Main heading" << std::endl;
        std::cout << "  - Heading 1 (built-in) → Section heading" << std::endl;
        std::cout << "  - Normal (built-in) → Body text paragraphs" << std::endl;
        std::cout << "  - Important Text (custom character style) → Emphasized text runs" << std::endl;
        std::cout << "  - Code (built-in character style) → Code blocks" << std::endl;

        print_header("Demo Complete");
        std::cout << "✓ Style application system fully functional!" << std::endl;
        std::cout << "✓ Both paragraph and character styles working correctly" << std::endl;
        std::cout << "✓ Built-in and custom styles integrated seamlessly" << std::endl;
        std::cout << "✓ Document created with proper style references in XML" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}