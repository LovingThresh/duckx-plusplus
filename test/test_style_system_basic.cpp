/*!
 * @file test_style_system_basic.cpp
 * @brief Basic test to verify style system works with corrected APIs
 */

#include <gtest/gtest.h>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Body.hpp"

using namespace duckx;

class BasicStyleSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto doc_result = Document::create_safe("test_style_system_basic.docx");
        ASSERT_TRUE(doc_result.ok());
        doc = std::make_unique<Document>(std::move(doc_result.value()));
        
        body = &doc->body();
        
        style_manager = std::make_unique<StyleManager>();
    }

    std::unique_ptr<Document> doc;
    Body* body;
    std::unique_ptr<StyleManager> style_manager;
};

TEST_F(BasicStyleSystemTest, BasicParagraphFormattingWorks)
{
    // Add a paragraph and apply direct formatting using existing methods
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Apply formatting using existing legacy methods
    para->set_alignment(Alignment::CENTER);
    para->set_spacing(12.0, 6.0);
    
    // Read properties using our new style reading system
    auto props_result = style_manager->read_paragraph_properties_safe(*para);
    EXPECT_TRUE(props_result.ok()) << "Should be able to read paragraph properties";
    
    if (props_result.ok()) {
        const auto& props = props_result.value();
        std::cout << "Read paragraph properties successfully!" << std::endl;
        
        if (props.alignment.has_value()) {
            std::cout << "  Alignment: " << static_cast<int>(props.alignment.value()) << std::endl;
        }
        if (props.space_before_pts.has_value()) {
            std::cout << "  Space before: " << props.space_before_pts.value() << " pts" << std::endl;
        }
        if (props.space_after_pts.has_value()) {
            std::cout << "  Space after: " << props.space_after_pts.value() << " pts" << std::endl;
        }
    }
}

TEST_F(BasicStyleSystemTest, BasicCharacterFormattingWorks)
{
    // Add a paragraph with a formatted run
    auto para_result = body->add_paragraph_safe("");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    // Create run with formatting using existing method
    duckx::Run& run = para->add_run("Formatted text", bold | italic);
    run.set_font("Arial").set_font_size(14.0).set_color("FF0000");
    
    // Read properties using our new style reading system
    auto props_result = style_manager->read_character_properties_safe(run);
    EXPECT_TRUE(props_result.ok()) << "Should be able to read character properties";
    
    if (props_result.ok()) {
        const auto& props = props_result.value();
        std::cout << "Read character properties successfully!" << std::endl;
        
        if (props.font_name.has_value()) {
            std::cout << "  Font: " << props.font_name.value() << std::endl;
        }
        if (props.font_size_pts.has_value()) {
            std::cout << "  Size: " << props.font_size_pts.value() << " pts" << std::endl;
        }
        if (props.font_color_hex.has_value()) {
            std::cout << "  Color: " << props.font_color_hex.value() << std::endl;
        }
    }
}

TEST_F(BasicStyleSystemTest, BasicTableFormattingWorks)
{
    // Add a table and apply direct formatting using existing methods
    auto table_result = body->add_table_safe(2, 2);
    ASSERT_TRUE(table_result.ok());
    Table* table = &table_result.value();
    
    // Apply formatting using existing legacy methods
    table->set_width(400.0);
    table->set_alignment("center");
    table->set_border_style("single");
    
    // Read properties using our new style reading system
    auto props_result = style_manager->read_table_properties_safe(*table);
    EXPECT_TRUE(props_result.ok()) << "Should be able to read table properties";
    
    if (props_result.ok()) {
        const auto& props = props_result.value();
        std::cout << "Read table properties successfully!" << std::endl;
        
        if (props.table_width_pts.has_value()) {
            std::cout << "  Width: " << props.table_width_pts.value() << " pts" << std::endl;
        }
        if (props.table_alignment.has_value()) {
            std::cout << "  Alignment: " << props.table_alignment.value() << std::endl;
        }
        if (props.border_style.has_value()) {
            std::cout << "  Border: " << props.border_style.value() << std::endl;
        }
    }
}

TEST_F(BasicStyleSystemTest, PropertyApplicationWorks)
{
    // Test applying properties through StyleManager
    auto para_result = body->add_paragraph_safe("Test paragraph");
    ASSERT_TRUE(para_result.ok());
    Paragraph* para = &para_result.value();
    
    ParagraphStyleProperties props;
    props.alignment = Alignment::CENTER;
    props.space_before_pts = 10.0;
    props.space_after_pts = 5.0;
    
    auto apply_result = style_manager->apply_paragraph_properties_safe(*para, props);
    EXPECT_TRUE(apply_result.ok()) << "Should be able to apply paragraph properties";
    
    if (apply_result.ok()) {
        std::cout << "Applied paragraph properties successfully!" << std::endl;
        
        // Verify by reading back
        auto read_result = style_manager->read_paragraph_properties_safe(*para);
        if (read_result.ok()) {
            const auto& read_props = read_result.value();
            if (read_props.alignment.has_value()) {
                EXPECT_EQ(Alignment::CENTER, read_props.alignment.value());
                std::cout << "  Verified alignment: CENTER" << std::endl;
            }
        }
    }
}