/*!
 * @file sample21_style_priority_test.cpp
 * @brief Style vs Direct Formatting Priority Test
 * 
 * This program demonstrates how styles and direct formatting interact,
 * showing that direct formatting overrides style settings.
 */

#include <iostream>
#include "duckx.hpp"
#include "test_utils.hpp"

using namespace duckx;

int main()
{
    try {
        std::cout << "Testing style vs direct formatting priority..." << std::endl;

        // Create document
        const std::string output_path = duckx::test_utils::get_temp_path("sample21_style_priority_test.docx");
        auto doc_result = Document::create_safe(output_path);
        if (!doc_result.ok()) {
            std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
            return 1;
        }
        Document doc = std::move(doc_result.value());
        auto& body = doc.body();
        auto& style_manager = doc.styles();
        
        // Load built-in styles
        style_manager.load_all_built_in_styles_safe();
        
        // Create a custom character style
        auto char_style_result = style_manager.create_character_style_safe("Blue Text");
        if (char_style_result.ok()) {
            Style* blue_style = char_style_result.value();
            CharacterStyleProperties props;
            props.font_name = "Times New Roman";
            props.font_size_pts = 14.0;
            props.font_color_hex = "0000FF";  // Blue
            blue_style->set_character_properties_safe(props);
            std::cout << "✓ Created 'Blue Text' character style (Times New Roman, 14pt, blue)" << std::endl;
        }

        std::cout << "\n=== Test 1: Style Only ===" << std::endl;
        auto para1_result = body.add_paragraph_safe("");
        if (para1_result.ok()) {
            Paragraph* para1 = &para1_result.value();
            para1->apply_style_safe(style_manager, "Normal");
            
            Run& run1 = para1->add_run("This text uses only the 'Blue Text' character style.");
            run1.apply_style_safe(style_manager, "Blue Text");
            
            std::cout << "✓ Applied style only - should be Times New Roman, 14pt, blue" << std::endl;
        }

        std::cout << "\n=== Test 2: Style + Direct Formatting Override ===" << std::endl;
        auto para2_result = body.add_paragraph_safe("");
        if (para2_result.ok()) {
            Paragraph* para2 = &para2_result.value();
            para2->apply_style_safe(style_manager, "Normal");
            
            Run& run2 = para2->add_run("This text has style + direct formatting override.");
            
            // First apply the style
            run2.apply_style_safe(style_manager, "Blue Text");
            
            // Then apply direct formatting (should override style)
            run2.set_font("Arial");           // Override font: Times New Roman → Arial
            run2.set_color("FF0000");         // Override color: Blue → Red
            // Note: size remains from style (14pt) since we don't override it
            
            std::cout << "✓ Applied style + direct overrides - should be Arial, 14pt, red" << std::endl;
        }

        std::cout << "\n=== Test 3: Direct Formatting Only ===" << std::endl;
        auto para3_result = body.add_paragraph_safe("");
        if (para3_result.ok()) {
            Paragraph* para3 = &para3_result.value();
            para3->apply_style_safe(style_manager, "Normal");
            
            Run& run3 = para3->add_run("This text uses only direct formatting.");
            
            // Apply direct formatting without any character style
            run3.set_font("Calibri");
            run3.set_font_size(16.0);
            run3.set_color("008000");  // Green
            
            std::cout << "✓ Applied direct formatting only - should be Calibri, 16pt, green" << std::endl;
        }

        std::cout << "\n=== Test 4: Paragraph Style + Character Style + Direct Formatting ===" << std::endl;
        auto para4_result = body.add_paragraph_safe("");
        if (para4_result.ok()) {
            Paragraph* para4 = &para4_result.value();
            
            // Apply paragraph style first
            para4->apply_style_safe(style_manager, "Heading 2");
            
            // Set paragraph-level direct formatting
            para4->set_alignment(Alignment::CENTER);  // Override alignment
            
            Run& run4 = para4->add_run("Complex formatting test with multiple layers.");
            
            // Apply character style
            run4.apply_style_safe(style_manager, "Blue Text");
            
            // Apply direct character formatting
            run4.set_color("800080");  // Purple (overrides blue from style)
            
            std::cout << "✓ Applied multiple formatting layers:" << std::endl;
            std::cout << "    - Paragraph style: Heading 2" << std::endl;
            std::cout << "    - Paragraph direct: centered alignment" << std::endl;
            std::cout << "    - Character style: Blue Text (Times New Roman, 14pt)" << std::endl;
            std::cout << "    - Character direct: purple color" << std::endl;
            std::cout << "    - Expected result: centered, Times New Roman, 14pt, purple" << std::endl;
        }

        // Save and report
        auto save_result = doc.save_safe();
        if (!save_result.ok()) {
            std::cerr << "Failed to save document: " << save_result.error().to_string() << std::endl;
            return 1;
        }
        
        std::cout << "\n✅ Document saved: " << output_path << std::endl;
        std::cout << "\n📋 DOCX Priority Rules (highest to lowest):" << std::endl;
        std::cout << "   1. Direct formatting (set_font, set_color, etc.) - HIGHEST" << std::endl;
        std::cout << "   2. Character styles (applied to runs)" << std::endl;
        std::cout << "   3. Paragraph styles (applied to paragraphs)" << std::endl;
        std::cout << "   4. Document defaults - LOWEST" << std::endl;
        std::cout << "\n🔍 Open the document in Word to verify the priority behavior!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}