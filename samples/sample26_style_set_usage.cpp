/*!
 * @file sample26_style_set_usage.cpp
 * @brief Demonstrates StyleSet functionality for batch style application
 * 
 * This sample shows how to:
 * - Create and register style sets
 * - Apply style sets to documents
 * - Use style mappings for intelligent style application
 * - Load style definitions from XML files
 * 
 * @date 2025.07
 */

#include <iostream>
#include <string>
#include <map>

#include "Document.hpp"
#include "StyleManager.hpp"
#include "XmlStyleParser.hpp"
#include "test_utils.hpp"

using namespace duckx;

int main()
{
    std::cout << "DuckX-PLusPlus StyleSet Usage Sample\n";
    std::cout << "=====================================\n\n";
    
    try {
        // Create a new document
        auto doc_result = Document::create_safe(test_utils::get_temp_path("style_set_demo.docx"));
        if (!doc_result.ok()) {
            std::cerr << "Failed to create document: " << doc_result.error().message() << std::endl;
            return 1;
        }
        
        auto& doc = doc_result.value();
        auto& body = doc.body();
        auto& style_mgr = doc.styles();
        
        std::cout << "1. Loading built-in styles...\n";
        auto load_result = style_mgr.load_all_built_in_styles_safe();
        if (!load_result.ok()) {
            std::cerr << "Failed to load built-in styles: " << load_result.error().message() << std::endl;
            return 1;
        }
        
        // ===== Example 1: Creating a Technical Documentation Style Set =====
        std::cout << "\n2. Creating Technical Documentation style set...\n";
        
        StyleSet tech_doc_set("TechnicalDocumentation");
        tech_doc_set.description = "Style set for technical documentation with code examples";
        tech_doc_set.included_styles = {
            "Heading 1",    // For main titles
            "Heading 2",    // For section titles
            "Heading 3",    // For subsections
            "Normal",       // For body text
            "Code"          // For code blocks
        };
        
        // Register the style set
        auto register_result = style_mgr.register_style_set_safe(tech_doc_set);
        if (!register_result.ok()) {
            std::cerr << "Failed to register style set: " << register_result.error().message() << std::endl;
            return 1;
        }
        
        std::cout << "   - Registered 'TechnicalDocumentation' style set\n";
        
        // ===== Example 2: Creating Document Content =====
        std::cout << "\n3. Creating document content...\n";
        
        // Add various content types
        auto p1 = body.add_paragraph_safe("DuckX-PLusPlus Technical Guide");
        if (p1.ok()) p1.value().apply_style_safe(doc.styles(), "Heading 1");
        
        auto p2 = body.add_paragraph_safe("Introduction");
        if (p2.ok()) p2.value().apply_style_safe(doc.styles(), "Heading 2");
        
        body.add_paragraph_safe("This guide demonstrates the StyleSet functionality for batch style application.");
        
        auto p3 = body.add_paragraph_safe("Getting Started");
        if (p3.ok()) p3.value().apply_style_safe(doc.styles(), "Heading 2");
        
        body.add_paragraph_safe("To use StyleSets, you first need to create and register them with the StyleManager.");
        
        auto p4 = body.add_paragraph_safe("Code Example");
        if (p4.ok()) p4.value().apply_style_safe(doc.styles(), "Heading 3");
        
        auto p5 = body.add_paragraph_safe("#include <Document.hpp>\n#include <StyleManager.hpp>\n\nauto doc = Document::create_safe(\"output.docx\");");
        if (p5.ok()) p5.value().apply_style_safe(doc.styles(), "Code");
        
        auto p6 = body.add_paragraph_safe("Advanced Features");
        if (p6.ok()) p6.value().apply_style_safe(doc.styles(), "Heading 2");
        
        body.add_paragraph_safe("StyleSets support cascading application and intelligent pattern matching.");
        
        // Add a table
        auto table_result = body.add_table_safe(3, 3);
        if (table_result.ok()) {
            auto& table = table_result.value();
            
            // Add table content using row/cell access
            auto rows = table.rows();
            if (rows.size() >= 3) {
                // Header row
                auto& row0 = *rows.begin();
                auto cells0 = row0.cells();
                auto cell_iter0 = cells0.begin();
                if (cell_iter0 != cells0.end()) {
                    cell_iter0->add_paragraph("Feature");
                    ++cell_iter0;
                }
                if (cell_iter0 != cells0.end()) {
                    cell_iter0->add_paragraph("Description");
                    ++cell_iter0;
                }
                if (cell_iter0 != cells0.end()) {
                    cell_iter0->add_paragraph("Status");
                }
                
                // Data rows
                auto row_iter = rows.begin();
                ++row_iter; // Move to second row
                if (row_iter != rows.end()) {
                    auto cells1 = row_iter->cells();
                    auto cell_iter1 = cells1.begin();
                    if (cell_iter1 != cells1.end()) {
                        cell_iter1->add_paragraph("StyleSet");
                        ++cell_iter1;
                    }
                    if (cell_iter1 != cells1.end()) {
                        cell_iter1->add_paragraph("Batch style application");
                        ++cell_iter1;
                    }
                    if (cell_iter1 != cells1.end()) {
                        cell_iter1->add_paragraph("Implemented");
                    }
                }
            }
        }
        
        // ===== Example 3: Applying Style Set =====
        std::cout << "\n4. Applying style set to document...\n";
        
        auto apply_result = doc.apply_style_set_safe("TechnicalDocumentation");
        if (!apply_result.ok()) {
            std::cerr << "Failed to apply style set: " << apply_result.error().message() << std::endl;
            return 1;
        }
        
        std::cout << "   - Applied 'TechnicalDocumentation' style set\n";
        
        // ===== Example 4: Creating a Business Report Style Set =====
        std::cout << "\n5. Creating Business Report style set...\n";
        
        // First create some custom styles for business reports
        auto heading_style = style_mgr.create_paragraph_style_safe("BusinessHeading");
        if (heading_style.ok()) {
            ParagraphStyleProperties heading_props;
            // Use basic properties (constants may not be available)
            heading_props.space_before_pts = 12.0;
            heading_props.space_after_pts = 6.0;
            heading_style.value()->set_paragraph_properties_safe(heading_props);
            
            CharacterStyleProperties heading_char_props;
            heading_char_props.font_name = "Arial";
            heading_char_props.font_size_pts = 16.0;
            // Skip formatting flags if constants aren't available
            heading_style.value()->set_character_properties_safe(heading_char_props);
        }
        
        auto body_style = style_mgr.create_paragraph_style_safe("BusinessBody");
        if (body_style.ok()) {
            ParagraphStyleProperties body_props;
            body_props.line_spacing = 1.15;
            body_style.value()->set_paragraph_properties_safe(body_props);
            
            CharacterStyleProperties body_char_props;
            body_char_props.font_name = "Times New Roman";
            body_char_props.font_size_pts = 11.0;
            body_style.value()->set_character_properties_safe(body_char_props);
        }
        
        // Create business report style set
        StyleSet business_set("BusinessReport");
        business_set.description = "Professional business report styling";
        business_set.included_styles = {
            "BusinessHeading",
            "BusinessBody",
            "Normal"
        };
        
        style_mgr.register_style_set_safe(business_set);
        std::cout << "   - Registered 'BusinessReport' style set\n";
        
        // ===== Example 5: Using Style Mappings =====
        std::cout << "\n6. Demonstrating style mappings...\n";
        
        // Create a mapping to apply custom styles based on patterns
        std::map<std::string, std::string> style_mappings = {
            {"heading1", "BusinessHeading"},  // Map all Heading 1 to BusinessHeading
            {"normal", "BusinessBody"},       // Map Normal paragraphs to BusinessBody
            {"code", "Code"}                  // Keep code blocks as-is
        };
        
        auto mapping_result = doc.apply_style_mappings_safe(style_mappings);
        if (!mapping_result.ok()) {
            std::cerr << "Failed to apply style mappings: " << mapping_result.error().message() << std::endl;
        } else {
            std::cout << "   - Applied custom style mappings\n";
        }
        
        // ===== Example 6: Listing Available Style Sets =====
        std::cout << "\n7. Listing available style sets...\n";
        
        auto style_sets = style_mgr.list_style_sets();
        std::cout << "   Available style sets:\n";
        for (const auto& set_name : style_sets) {
            auto set_result = style_mgr.get_style_set_safe(set_name);
            if (set_result.ok()) {
                const auto& style_set = set_result.value();
                std::cout << "   - " << set_name << ": " << style_set.description << "\n";
                std::cout << "     Includes " << style_set.included_styles.size() << " styles\n";
            }
        }
        
        // ===== Save the Document =====
        std::cout << "\n8. Saving document...\n";
        
        auto save_result = doc.save_safe();
        if (!save_result.ok()) {
            std::cerr << "Failed to save document: " << save_result.error().message() << std::endl;
            return 1;
        }
        
        std::cout << "   - Document saved successfully to temp directory\n";
        
        // ===== Example 7: Loading Style Definitions from XML (if available) =====
        std::cout << "\n9. Example: Loading styles from XML file...\n";
        std::cout << "   (This would load from an XML file if provided)\n";
        
        // Example XML content that could be loaded:
        std::cout << "\n   Example XML style definition:\n";
        std::cout << "   <?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        std::cout << "   <styles xmlns=\"http://duckx.org/styles\" version=\"1.0\">\n";
        std::cout << "     <style name=\"CustomTitle\" type=\"paragraph\">\n";
        std::cout << "       <paragraph>\n";
        std::cout << "         <alignment>center</alignment>\n";
        std::cout << "         <spacing before=\"24pt\" after=\"12pt\"/>\n";
        std::cout << "       </paragraph>\n";
        std::cout << "       <character>\n";
        std::cout << "         <font name=\"Calibri\" size=\"20pt\" color=\"#1F4788\"/>\n";
        std::cout << "         <formatting bold=\"true\"/>\n";
        std::cout << "       </character>\n";
        std::cout << "     </style>\n";
        std::cout << "     \n";
        std::cout << "     <styleSet name=\"CustomTheme\">\n";
        std::cout << "       <description>Custom document theme</description>\n";
        std::cout << "       <includes>\n";
        std::cout << "         <style>CustomTitle</style>\n";
        std::cout << "         <style>Normal</style>\n";
        std::cout << "       </includes>\n";
        std::cout << "     </styleSet>\n";
        std::cout << "   </styles>\n";
        
        // To actually load from XML:
        // auto load_result = doc.load_style_definitions_safe("custom_styles.xml");
        
        std::cout << "\n=====================================\n";
        std::cout << "StyleSet demonstration complete!\n";
        std::cout << "Check the temp directory for 'style_set_demo.docx' to see the results.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}