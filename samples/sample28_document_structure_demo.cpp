/*!
 * @file sample28_document_structure_demo.cpp
 * @brief Comprehensive demonstration of document structure enhancement features
 * 
 * This sample demonstrates:
 * - Document outline generation based on heading styles
 * - Table of contents creation with multi-level support
 * - Page layout management (margins, orientation, sections)
 * - Section breaks and page formatting
 * - Integration of all document structure features
 * 
 * @date 2025.07
 */

#include <iostream>
#include <string>
#include "Document.hpp"
#include "OutlineManager.hpp"
#include "PageLayoutManager.hpp"
#include "StyleManager.hpp"
#include "test_utils.hpp"

using namespace duckx;

int main() {
    try {
        std::cout << "=== Document Structure Enhancement Demo ===" << std::endl;
        
        // Create output file path using unified test utilities
        std::string output_path = test_utils::get_temp_path("sample28_document_structure_demo.docx");
        
        // Create new document
        std::cout << "Creating new document..." << std::endl;
        auto doc_result = Document::create_safe(output_path);
        if (!doc_result.ok()) {
            std::cerr << "Failed to create document: " << doc_result.error().message() << std::endl;
            return 1;
        }
        
        auto doc = std::move(doc_result.value());
        
        // ===== STEP 0: Initialize Page Layout Structure =====
        std::cout << "Initializing page layout structure..." << std::endl;
        auto init_layout_result = doc.initialize_page_layout_structure_safe();
        if (!init_layout_result.ok()) {
            std::cerr << "Failed to initialize page layout: " << init_layout_result.error().message() << std::endl;
            return 1;
        }
        
        // Get managers
        auto& body = doc.body();
        auto& styles = doc.styles();
        auto& outline = doc.outline();
        auto& layout = doc.page_layout();
        
        // ===== STEP 1: Setup Page Layout (simplified) =====
        std::cout << "Setting up basic document..." << std::endl;
        
        // ===== STEP 2: Create Heading Styles =====
        std::cout << "Creating heading styles..." << std::endl;
        
        // Create Heading 1 style
        auto h1_style_result = styles.create_paragraph_style_safe("Heading 1");
        if (h1_style_result.ok()) {
            auto h1_style = h1_style_result.value();
            h1_style->set_font_safe("Arial", 16.0);
            h1_style->set_color_safe("2F5496");
            h1_style->set_spacing_safe(12.0, 6.0);
            
            // Set bold formatting using character properties
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
            
            // Set bold formatting using character properties
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
            
            // Set bold formatting using character properties
            CharacterStyleProperties char_props;
            char_props.formatting_flags = bold;
            h3_style->set_character_properties_safe(char_props);
            
            std::cout << "✓ Created Heading 3 style" << std::endl;
        } else {
            std::cerr << "⚠ Failed to create Heading 3 style: " << h3_style_result.error().message() << std::endl;
        }
        
        // ===== STEP 3: Create Document Content =====
        std::cout << "Creating document content..." << std::endl;
        
        // Title page content
        auto title_result = body.add_paragraph_safe("Advanced Document Structure");
        if (title_result.ok()) {
            styles.apply_paragraph_style_safe(title_result.value(), "Document Title");
        }
        
        auto subtitle_result = body.add_paragraph_safe("Demonstration of Enhanced Features");
        if (subtitle_result.ok()) {
            auto subtitle_style_result = styles.create_paragraph_style_safe("Subtitle");
            if (subtitle_style_result.ok()) {
                auto subtitle_style = subtitle_style_result.value();
                subtitle_style->set_font_safe("Arial", 16.0);
                subtitle_style->set_alignment_safe(Alignment::CENTER);
                styles.apply_paragraph_style_safe(subtitle_result.value(), "Subtitle");
            }
        }
        
        body.add_paragraph_safe("");  // Empty line
        body.add_paragraph_safe("This document demonstrates the enhanced document structure features including automated outline generation, table of contents creation, and advanced page layout management.");
        
        // Insert page break to start content on new page
        auto page_break_result = layout.insert_section_break_safe(SectionBreakType::NEXT_PAGE);
        if (!page_break_result.ok()) {
            std::cerr << "Failed to insert page break: " << page_break_result.error().message() << std::endl;
        }
        
        // ===== STEP 4: Create Table of Contents =====
        std::cout << "Creating table of contents..." << std::endl;
        
        TocOptions toc_options;
        toc_options.toc_title = "Table of Contents";
        toc_options.max_level = 3;
        toc_options.show_page_numbers = true;
        toc_options.use_hyperlinks = true;
        toc_options.right_align_page_numbers = true;
        toc_options.leader_char = ".";
        
        auto toc_result = outline.create_toc_safe(toc_options);
        if (!toc_result.ok()) {
            std::cerr << "Failed to create TOC: " << toc_result.error().message() << std::endl;
        }
        
        body.add_paragraph_safe("");  // Empty line after TOC
        
        // Insert another page break
        layout.insert_section_break_safe(SectionBreakType::NEXT_PAGE);
        
        // ===== STEP 5: Main Document Content with Headings =====
        std::cout << "Adding main content with structured headings..." << std::endl;
        
        // Chapter 1: Introduction
        auto h1_intro_result = body.add_paragraph_safe("1. Introduction");
        if (h1_intro_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h1_intro_result.value(), "Heading 1");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Introduction'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 1 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("This document serves as a comprehensive demonstration of the document structure enhancement features implemented in the DuckX library. These features provide automated document organization and professional formatting capabilities.");
        
        auto h2_purpose_result = body.add_paragraph_safe("1.1 Purpose and Scope");
        if (h2_purpose_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h2_purpose_result.value(), "Heading 2");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Purpose and Scope'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 2 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("The purpose of this demonstration is to showcase the integrated document structure features including outline generation, table of contents creation, and advanced page layout management.");
        
        auto h3_features_result = body.add_paragraph_safe("1.1.1 Key Features");
        if (h3_features_result.ok()) {
            styles.apply_paragraph_style_safe(h3_features_result.value(), "Heading 3");
        }
        
        body.add_paragraph_safe("• Automated outline generation based on heading styles");
        body.add_paragraph_safe("• Multi-level table of contents with page numbers");
        body.add_paragraph_safe("• Advanced page layout and section management");
        body.add_paragraph_safe("• Professional document formatting");
        
        // Chapter 2: Outline Management
        auto h1_outline_result = body.add_paragraph_safe("2. Document Outline Management");
        if (h1_outline_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h1_outline_result.value(), "Heading 1");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Document Outline Management'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 1 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("The OutlineManager provides sophisticated document structure analysis and table of contents generation capabilities.");
        
        auto h2_generation_result = body.add_paragraph_safe("2.1 Outline Generation");
        if (h2_generation_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h2_generation_result.value(), "Heading 2");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Outline Generation'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 2 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("The outline generation process analyzes the document structure based on heading styles and creates a hierarchical representation of the document content.");
        
        auto h2_toc_result = body.add_paragraph_safe("2.2 Table of Contents Creation");
        if (h2_toc_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h2_toc_result.value(), "Heading 2");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Table of Contents Creation'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 2 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("Tables of contents are automatically generated with configurable formatting options including page numbers, hyperlinks, and visual styling.");
        
        // Chapter 3: Page Layout Management
        auto h1_layout_result = body.add_paragraph_safe("3. Page Layout and Section Management");
        if (h1_layout_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h1_layout_result.value(), "Heading 1");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Page Layout and Section Management'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 1 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("The PageLayoutManager provides comprehensive control over document layout including page size, orientation, margins, and section breaks.");
        
        auto h2_page_setup_result = body.add_paragraph_safe("3.1 Page Setup and Formatting");
        if (h2_page_setup_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h2_page_setup_result.value(), "Heading 2");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Page Setup and Formatting'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 2 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("Page formatting includes support for standard page sizes (A4, Letter, Legal), custom dimensions, and both portrait and landscape orientations.");
        
        auto h3_margins_result = body.add_paragraph_safe("3.1.1 Margin Configuration");
        if (h3_margins_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h3_margins_result.value(), "Heading 3");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 3 style to 'Margin Configuration'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 3 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("Margins can be configured individually for top, bottom, left, right, header, and footer areas with precise millimeter control.");
        
        // Insert section break and change to landscape for demonstration
        std::cout << "Inserting section break and changing to landscape..." << std::endl;
        layout.insert_section_break_safe(SectionBreakType::NEXT_PAGE);
        
        // Change to landscape orientation for next section
        auto landscape_result = layout.set_orientation_safe(PageOrientation::LANDSCAPE);
        if (!landscape_result.ok()) {
            std::cerr << "Failed to set landscape orientation: " << landscape_result.error().message() << std::endl;
        }
        
        // Set two-column layout
        auto columns_result = layout.set_columns_safe(2, 12.0);
        if (!columns_result.ok()) {
            std::cerr << "Failed to set columns: " << columns_result.error().message() << std::endl;
        }
        
        auto h2_sections_result = body.add_paragraph_safe("3.2 Section Management");
        if (h2_sections_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h2_sections_result.value(), "Heading 2");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Section Management'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 2 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("This section demonstrates landscape orientation and two-column layout. Section breaks allow different formatting within the same document.");
        
        body.add_paragraph_safe("The section management system supports various break types including next page, even page, odd page, and continuous breaks. Each section can have its own page setup, margins, and column configuration.");
        
        auto h3_columns_result = body.add_paragraph_safe("3.2.1 Column Layout");
        if (h3_columns_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h3_columns_result.value(), "Heading 3");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 3 style to 'Column Layout'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 3 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("Multi-column layouts support up to 10 columns with configurable spacing. This provides flexibility for creating newsletters, brochures, and other multi-column documents.");
        
        // Return to portrait and single column
        layout.insert_section_break_safe(SectionBreakType::NEXT_PAGE);
        layout.set_orientation_safe(PageOrientation::PORTRAIT);
        layout.set_columns_safe(1);
        
        // Chapter 4: Integration and Best Practices
        auto h1_integration_result = body.add_paragraph_safe("4. Integration and Best Practices");
        if (h1_integration_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h1_integration_result.value(), "Heading 1");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Integration and Best Practices'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 1 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("The document structure enhancement features work together to provide a comprehensive solution for professional document creation.");
        
        auto h2_workflow_result = body.add_paragraph_safe("4.1 Recommended Workflow");
        if (h2_workflow_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h2_workflow_result.value(), "Heading 2");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 2 style to 'Recommended Workflow'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 2 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("1. Set up page layout and margins");
        body.add_paragraph_safe("2. Create and configure heading styles");
        body.add_paragraph_safe("3. Add document content with proper heading structure");
        body.add_paragraph_safe("4. Generate outline and create table of contents");
        body.add_paragraph_safe("5. Apply section breaks and formatting as needed");
        
        // Chapter 5: Conclusion
        auto h1_conclusion_result = body.add_paragraph_safe("5. Conclusion");
        if (h1_conclusion_result.ok()) {
            auto style_result = styles.apply_paragraph_style_safe(h1_conclusion_result.value(), "Heading 1");
            if (style_result.ok()) {
                std::cout << "✓ Applied Heading 1 style to 'Conclusion'" << std::endl;
            } else {
                std::cerr << "⚠ Failed to apply Heading 1 style: " << style_result.error().message() << std::endl;
            }
        }
        
        body.add_paragraph_safe("The document structure enhancement features provide powerful tools for creating professional, well-organized documents with automated navigation and consistent formatting.");
        
        // ===== STEP 6: Generate and Update Outline =====
        std::cout << "Generating document outline..." << std::endl;
        
        auto outline_result = outline.generate_outline_safe();
        if (outline_result.ok()) {
            const auto& outline_entries = outline_result.value();
            std::cout << "Generated outline with " << outline_entries.size() << " entries:" << std::endl;
            
            // Export outline as text for console display
            auto text_export_result = outline.export_outline_as_text_safe(2);
            if (text_export_result.ok()) {
                std::cout << text_export_result.value() << std::endl;
            }
        } else {
            std::cerr << "Failed to generate outline: " << outline_result.error().message() << std::endl;
        }
        
        // Calculate page numbers (simplified estimation)
        auto page_calc_result = outline.calculate_page_numbers_safe();  
        if (!page_calc_result.ok()) {
            std::cerr << "Failed to calculate page numbers: " << page_calc_result.error().message() << std::endl;
        }
        
        // ===== STEP 7: Save Document =====
        std::cout << "Saving document..." << std::endl;
        
        auto save_result = doc.save_safe();
        if (!save_result.ok()) {
            std::cerr << "Failed to save document: " << save_result.error().message() << std::endl;
            return 1;
        }
        
        std::cout << "Document saved successfully as: " << output_path << std::endl;
        
        // ===== STEP 8: Display Summary =====
        std::cout << "\n=== Document Structure Summary ===" << std::endl;
        
        // Display page layout information
        auto current_margins_result = layout.get_margins_safe();
        if (current_margins_result.ok()) {
            const auto& current_margins = current_margins_result.value();
            std::cout << "Final margins: " 
                      << current_margins.top_mm << "mm (top), "
                      << current_margins.bottom_mm << "mm (bottom), "
                      << current_margins.left_mm << "mm (left), "
                      << current_margins.right_mm << "mm (right)" << std::endl;
        }
        
        auto page_size_result = layout.get_page_size_safe();
        if (page_size_result.ok()) {
            const auto& page_config = page_size_result.value();
            std::cout << "Final page size: " 
                      << page_config.width_mm << "mm x " << page_config.height_mm << "mm ("
                      << (page_config.orientation == PageOrientation::PORTRAIT ? "Portrait" : "Landscape")
                      << ")" << std::endl;
        }
        
        std::cout << "Section count: " << layout.get_section_count() << std::endl;
        
        auto column_count_result = layout.get_column_count_safe();
        if (column_count_result.ok()) {
            std::cout << "Current columns: " << column_count_result.value() << std::endl;
        }
        
        std::cout << "\n=== Demo Complete ===" << std::endl;
        std::cout << "Open '" << output_path << "' to view the results!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}