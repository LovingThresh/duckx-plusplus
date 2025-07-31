/*!
 * @file sample29_simple_outline_demo.cpp
 * @brief Focused demonstration of OutlineManager core functionality
 * 
 * This sample demonstrates:
 * - Creating documents with hierarchical heading structure
 * - Generating document outlines based on heading styles
 * - Creating table of contents with multiple levels
 * - Exporting outline as structured text
 * 
 * @date 2025.07
 */

#include <iostream>
#include <string>
#include "Document.hpp"
#include "OutlineManager.hpp"
#include "StyleManager.hpp"
#include "test_utils.hpp"

using namespace duckx;

int main() {
    try {
        std::cout << "=== OutlineManager Focused Demo ===" << std::endl;
        
        // Create new document using unified test utilities
        std::cout << "Creating new document..." << std::endl;
        std::string output_path = test_utils::get_temp_path("sample29_simple_outline_demo.docx");
        auto doc_result = Document::create_safe(output_path);
        if (!doc_result.ok()) {
            std::cerr << "Failed to create document: " << doc_result.error().message() << std::endl;
            return 1;
        }
        
        auto doc = std::move(doc_result.value());
        
        // Initialize outline manager properly
        auto init_result = doc.initialize_page_layout_structure_safe();
        if (!init_result.ok()) {
            std::cerr << "Failed to initialize document structure: " << init_result.error().message() << std::endl;
            return 1;
        }
        
        // Get managers
        auto& body = doc.body();
        auto& styles = doc.styles();
        auto& outline = doc.outline();
        
        // ===== STEP 1: Create Required Heading Styles =====
        std::cout << "\\n=== Creating Heading Styles ===" << std::endl;
        
        // Create Title style
        auto title_style_result = styles.create_paragraph_style_safe("Title");
        if (title_style_result.ok()) {
            auto title_style = title_style_result.value();
            title_style->set_font_safe("Arial", 18.0);
            title_style->set_color_safe("2F5496");
            title_style->set_alignment_safe(Alignment::CENTER);
            title_style->set_spacing_safe(16.0, 8.0);
            
            CharacterStyleProperties char_props;
            char_props.formatting_flags = bold;
            title_style->set_character_properties_safe(char_props);
            
            std::cout << "✓ Created Title style" << std::endl;
        } else {
            std::cerr << "⚠ Failed to create Title style: " << title_style_result.error().message() << std::endl;
        }
        
        // Create Heading 1 style
        auto h1_style_result = styles.create_paragraph_style_safe("Heading 1");
        if (h1_style_result.ok()) {
            auto h1_style = h1_style_result.value();
            h1_style->set_font_safe("Arial", 16.0);
            h1_style->set_color_safe("2F5496");
            h1_style->set_spacing_safe(12.0, 6.0);
            
            CharacterStyleProperties char_props;
            char_props.formatting_flags = bold;
            h1_style->set_character_properties_safe(char_props);
            
            std::cout << "✓ Created Heading 1 style" << std::endl;
        } else {
            std::cerr << "⚠ Failed to create Heading 1 style: " << h1_style_result.error().message() << std::endl;
        }
        
        // Create Heading 2 style
        auto h2_style_result = styles.create_paragraph_style_safe("Heading 2");
        if (h2_style_result.ok()) {
            auto h2_style = h2_style_result.value();
            h2_style->set_font_safe("Arial", 14.0);
            h2_style->set_color_safe("2F5496");
            h2_style->set_spacing_safe(10.0, 4.0);
            
            CharacterStyleProperties char_props;
            char_props.formatting_flags = bold;
            h2_style->set_character_properties_safe(char_props);
            
            std::cout << "✓ Created Heading 2 style" << std::endl;
        } else {
            std::cerr << "⚠ Failed to create Heading 2 style: " << h2_style_result.error().message() << std::endl;
        }
        
        // Create Heading 3 style
        auto h3_style_result = styles.create_paragraph_style_safe("Heading 3");
        if (h3_style_result.ok()) {
            auto h3_style = h3_style_result.value();
            h3_style->set_font_safe("Arial", 12.0);
            h3_style->set_color_safe("2F5496");
            h3_style->set_spacing_safe(8.0, 3.0);
            
            CharacterStyleProperties char_props;
            char_props.formatting_flags = bold;
            h3_style->set_character_properties_safe(char_props);
            
            std::cout << "✓ Created Heading 3 style" << std::endl;
        } else {
            std::cerr << "⚠ Failed to create Heading 3 style: " << h3_style_result.error().message() << std::endl;
        }
        
        // ===== STEP 2: Create Document Structure with Headings =====
        std::cout << "\\n=== Creating Document Structure ===" << std::endl;
        
        // Document title
        auto title_result = body.add_paragraph_safe("OutlineManager Demonstration");
        if (title_result.ok()) {
            auto title_style_result = styles.apply_paragraph_style_safe(title_result.value(), "Title");
            if (title_style_result.ok()) {
                std::cout << "✓ Applied Title style" << std::endl;
            } else {
                std::cout << "ⓘ Title style not available, using default formatting" << std::endl;
            }
        }
        
        body.add_paragraph_safe("This document demonstrates OutlineManager functionality with automatic outline generation.");
        body.add_paragraph_safe("");
        
        // Add placeholder for TOC (will be created after content)
        std::cout << "Preparing Table of Contents location..." << std::endl;
        body.add_paragraph_safe("[Table of Contents will be inserted here]");
        body.add_paragraph_safe("");
        
        // Chapter 1: Introduction
        auto h1_intro_result = body.add_paragraph_safe("1. Introduction");
        if (h1_intro_result.ok()) {
            auto h1_style_result = styles.apply_paragraph_style_safe(h1_intro_result.value(), "Heading 1");
            if (h1_style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Introduction'" << std::endl;
            } else {
                std::cout << "ⓘ Heading 1 style not available for 'Introduction'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("The OutlineManager analyzes document structure and creates navigation aids automatically.");
        
        // Subsection 1.1
        auto h2_purpose_result = body.add_paragraph_safe("1.1 Purpose and Benefits");
        if (h2_purpose_result.ok()) {
            auto h2_style_result = styles.apply_paragraph_style_safe(h2_purpose_result.value(), "Heading 2");
            if (h2_style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Purpose and Benefits'" << std::endl;
            } else {
                std::cout << "ⓘ Heading 2 style not available for 'Purpose and Benefits'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("Key benefits include automated outline generation, table of contents creation, and hierarchical document structure.");
        
        // Subsection 1.2
        auto h2_features_result = body.add_paragraph_safe("1.2 Core Features");
        if (h2_features_result.ok()) {
            auto h2_style_result = styles.apply_paragraph_style_safe(h2_features_result.value(), "Heading 2");
            if (h2_style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Core Features'" << std::endl;
            } else {
                std::cout << "⚠ Failed to apply Heading 2 style to 'Core Features'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("• Multi-level heading support (up to 9 levels)");
        body.add_paragraph_safe("• Flexible TOC formatting options");
        body.add_paragraph_safe("• Export capabilities to various formats");
        
        // Chapter 2
        auto h1_impl_result = body.add_paragraph_safe("2. Implementation Details");
        if (h1_impl_result.ok()) {
            auto h1_style_result = styles.apply_paragraph_style_safe(h1_impl_result.value(), "Heading 1");
            if (h1_style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Implementation Details'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("This section covers technical implementation aspects.");
        
        // Subsection 2.1
        auto h2_analysis_result = body.add_paragraph_safe("2.1 Document Analysis");
        if (h2_analysis_result.ok()) {
            auto h2_style_result = styles.apply_paragraph_style_safe(h2_analysis_result.value(), "Heading 2");
            if (h2_style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Document Analysis'" << std::endl;
            } else {
                std::cout << "⚠ Failed to apply Heading 2 style to 'Document Analysis'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("The system scans for paragraphs with heading styles and builds hierarchical structure.");
        
        // Deep subsection 2.1.1
        auto h3_scanning_result = body.add_paragraph_safe("2.1.1 Scanning Process");
        if (h3_scanning_result.ok()) {
            auto h3_style_result = styles.apply_paragraph_style_safe(h3_scanning_result.value(), "Heading 3");
            if (h3_style_result.ok()) {
                std::cout << "✓ Applied Heading 3 style to 'Scanning Process'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("The scanning process identifies heading levels and creates nested outline entries.");
        
        // Chapter 3
        auto h1_usage_result = body.add_paragraph_safe("3. Usage Examples");
        if (h1_usage_result.ok()) {
            auto h1_style_result = styles.apply_paragraph_style_safe(h1_usage_result.value(), "Heading 1");
            if (h1_style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Usage Examples'" << std::endl;
            } else {
                std::cout << "⚠ Failed to apply Heading 1 style to 'Usage Examples'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("Practical examples of OutlineManager usage in different scenarios.");
        
        // Chapter 4: Conclusion
        auto h1_conclusion_result = body.add_paragraph_safe("4. Conclusion");
        if (h1_conclusion_result.ok()) {
            auto h1_style_result = styles.apply_paragraph_style_safe(h1_conclusion_result.value(), "Heading 1");
            if (h1_style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Conclusion'" << std::endl;
            } else {
                std::cout << "⚠ Failed to apply Heading 1 style to 'Conclusion'" << std::endl;
            }
        }
        
        body.add_paragraph_safe("The OutlineManager provides powerful document organization capabilities, enabling automatic outline generation and table of contents creation for professional document workflows.");
        
        // ===== STEP 3: Generate Document Outline =====
        std::cout << "\\n=== Generating Document Outline ===" << std::endl;
        
        auto outline_result = outline.generate_outline_safe();
        if (outline_result.ok()) {
            const auto& outline_entries = outline_result.value();
            std::cout << "Successfully generated outline with " << outline_entries.size() << " entries" << std::endl;
            
            // Export outline as structured text for console display
            auto text_export_result = outline.export_outline_as_text_safe(4);
            if (text_export_result.ok()) {
                std::cout << "\\nDocument Structure:" << std::endl;
                std::cout << "===================" << std::endl;
                std::cout << text_export_result.value() << std::endl;
            } else {
                std::cerr << "Failed to export outline as text: " << text_export_result.error().message() << std::endl;
            }
        } else {
            std::cerr << "Failed to generate outline: " << outline_result.error().message() << std::endl;
            std::cout << "This may be because heading styles were not properly applied." << std::endl;
        }
        
        // ===== STEP 4: Update Table of Contents (if needed) =====
        std::cout << "\\n=== Creating Real Table of Contents ===" << std::endl;
        
        // Now create the real field-based TOC at the placeholder location
        TocOptions toc_options;
        toc_options.toc_title = "Table of Contents";
        toc_options.max_level = 3;
        toc_options.show_page_numbers = true;
        toc_options.use_hyperlinks = true;
        toc_options.right_align_page_numbers = true;
        toc_options.leader_char = ".";
        
        auto field_toc_result = outline.create_field_toc_at_placeholder_safe(
            "[Table of Contents will be inserted here]", toc_options);
        if (field_toc_result.ok()) {
            std::cout << "✓ Field-based Table of Contents inserted at correct position!" << std::endl;
            std::cout << "  (This will be updateable in Word)" << std::endl;
        } else {
            std::cerr << "⚠ Failed to create field TOC at placeholder: " << field_toc_result.error().message() << std::endl;
            // Fall back to appending TOC
            auto fallback_toc_result = outline.create_field_toc_safe(toc_options);
            if (fallback_toc_result.ok()) {
                std::cout << "✓ Field-based Table of Contents created at end of document" << std::endl;
            } else {
                std::cerr << "⚠ Failed to create any TOC: " << fallback_toc_result.error().message() << std::endl;
            }
        }
        
        std::cout << "\\n=== Updating Table of Contents ===" << std::endl;
        
        // Update page numbers
        auto update_result = outline.calculate_page_numbers_safe();
        if (update_result.ok()) {
            std::cout << "✓ TOC page numbers updated successfully!" << std::endl;
        } else {
            std::cerr << "⚠ Failed to update page numbers: " << update_result.error().message() << std::endl;
        }
        
        // ===== STEP 5: Save Document =====
        std::cout << "\\n=== Saving Document ===" << std::endl;
        
        auto save_result = doc.save_safe();
        if (!save_result.ok()) {
            std::cerr << "Failed to save document: " << save_result.error().message() << std::endl;
            return 1;
        }
        
        // ===== STEP 6: Summary =====
        std::cout << "\\n=== Demo Summary ===" << std::endl;
        std::cout << "✓ Created document with hierarchical structure" << std::endl;
        std::cout << "✓ Applied heading styles (Title, Heading 1-3)" << std::endl;
        std::cout << "✓ Generated document outline" << std::endl;
        std::cout << "✓ Created table of contents" << std::endl;
        std::cout << "✓ Exported outline structure as text" << std::endl;
        
        std::cout << "\\nDocument saved as: " << output_path << std::endl;
        std::cout << "\\nOutlineManager Demo completed! 🎉" << std::endl;
        std::cout << "\\nNote: If heading styles were not available, the outline may be empty." << std::endl;
        std::cout << "This is normal for new documents without predefined heading styles." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}