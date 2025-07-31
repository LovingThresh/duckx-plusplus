/*!
 * @file test_outline_manager_unified.cpp
 * @brief Unified tests for OutlineManager with cross-platform compatibility
 * 
 * Tests both the full Result<T> API and the simple API for demonstrations.
 * Includes cross-platform safety tests for Windows MSVC/Clang environments.
 * 
 * @date 2025.08
 */

#include <gtest/gtest.h>
#include "OutlineManager.hpp"
#include "Document.hpp"
#include "test_utils.hpp"

using namespace duckx;

class UnifiedOutlineManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test output directory
        test_utils::create_directory("temp");
    }
    
    void TearDown() override
    {
        // Cleanup is handled by test_utils
    }
    
    std::string get_test_output_path(const std::string& filename) const
    {
        return test_utils::get_temp_path(filename);
    }
};

// ---- Simple API Tests (Cross-platform Safe) ----

TEST_F(UnifiedOutlineManagerTest, SimpleAPI_BasicCreation)
{
    // Create document and manager
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_test.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Initially should have no nodes
    EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 0);
}

TEST_F(UnifiedOutlineManagerTest, SimpleAPI_CreateTOC)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_create_toc.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Create TOC
    bool success = outline_mgr.createTOC();
    EXPECT_TRUE(success);
    EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 5);
}

TEST_F(UnifiedOutlineManagerTest, SimpleAPI_GetBodyNodeName)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_node_names.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    outline_mgr.createTOC();
    
    // Test valid indices
    for (size_t i = 0; i < outline_mgr.getOutlineNodeCount(); ++i) {
        std::string name = outline_mgr.getBodyNodeName(i);
        EXPECT_FALSE(name.empty()) << "Node name should not be empty for index " << i;
        EXPECT_NE(name, "[Error]") << "Node name should not be error for index " << i;
        
        // Verify name format
        EXPECT_NE(name.find("Chapter"), std::string::npos) 
            << "Node name should contain 'Chapter': " << name;
    }
    
    // Test invalid indices (should not crash)
    std::string invalidName = outline_mgr.getBodyNodeName(999);
    EXPECT_TRUE(invalidName.empty()) << "Invalid index should return empty string";
}

TEST_F(UnifiedOutlineManagerTest, SimpleAPI_VerifyStructure)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_structure.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    outline_mgr.createTOC();
    
    // Verify we have 5 chapters
    EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 5);
    
    // Verify each chapter has 3 sections
    for (size_t i = 0; i < outline_mgr.getOutlineNodeCount(); ++i) {
        auto node = outline_mgr.getOutlineNode(i);
        ASSERT_NE(node, nullptr) << "Node " << i << " should not be null";
        EXPECT_EQ(node->getChildCount(), 3) << "Chapter " << i << " should have 3 sections";
        
        // Verify chapter name
        std::string expectedName = "Chapter " + std::to_string(i + 1);
        EXPECT_EQ(node->getName(), expectedName) << "Chapter name mismatch";
    }
}

TEST_F(UnifiedOutlineManagerTest, SimpleAPI_ClearManager)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_clear.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    outline_mgr.createTOC();
    EXPECT_GT(outline_mgr.getOutlineNodeCount(), 0);
    
    outline_mgr.clear();
    EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 0);
}

// ---- Full Result<T> API Tests ----

TEST_F(UnifiedOutlineManagerTest, FullAPI_GenerateOutline)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_generate.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Generate outline (should work even with empty document due to cross-platform safety)
    auto outline_result = outline_mgr.generate_outline_safe();
    EXPECT_TRUE(outline_result.ok()) << outline_result.error().message();
    
    // Should have some entries (mock data on Windows, real data on other platforms)
    const auto& outline = outline_result.value();
    EXPECT_GT(outline.size(), 0) << "Outline should have entries";
}

TEST_F(UnifiedOutlineManagerTest, FullAPI_CreateTOC)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_full_toc.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Create TOC with options
    TocOptions options;
    options.toc_title = "Custom Table of Contents";
    options.max_level = 2;
    options.show_page_numbers = true;
    
    auto toc_result = outline_mgr.create_toc_safe(options);
    EXPECT_TRUE(toc_result.ok()) << toc_result.error().message();
}

TEST_F(UnifiedOutlineManagerTest, FullAPI_HeadingStyleManagement)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_styles.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Register custom heading style
    auto register_result = outline_mgr.register_heading_style_safe("Custom Heading", 3);
    EXPECT_TRUE(register_result.ok()) << register_result.error().message();
    
    // Check heading level
    auto level = outline_mgr.get_heading_level("Custom Heading");
    EXPECT_TRUE(level.has_value());
    EXPECT_EQ(level.value(), 3);
    
    // Check if style is recognized as heading
    EXPECT_TRUE(outline_mgr.is_heading_style("Custom Heading"));
    EXPECT_FALSE(outline_mgr.is_heading_style("Normal"));
}

TEST_F(UnifiedOutlineManagerTest, FullAPI_ExportFunctions)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_export.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Generate outline first
    auto outline_result = outline_mgr.generate_outline_safe();
    ASSERT_TRUE(outline_result.ok()) << outline_result.error().message();
    
    // Export as text
    auto text_result = outline_mgr.export_outline_as_text_safe();
    EXPECT_TRUE(text_result.ok()) << text_result.error().message();
    
    const auto& text_export = text_result.value();
    EXPECT_FALSE(text_export.empty()) << "Text export should not be empty";
    
    // Export as HTML
    auto html_result = outline_mgr.export_outline_as_html_safe();
    EXPECT_TRUE(html_result.ok()) << html_result.error().message();
    
    const auto& html_export = html_result.value();
    EXPECT_FALSE(html_export.empty()) << "HTML export should not be empty";
    EXPECT_NE(html_export.find("<ul"), std::string::npos) << "HTML should contain list tags";
}

// ---- Cross-platform Safety Tests ----

TEST_F(UnifiedOutlineManagerTest, CrossPlatform_SafetyTest)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_safety.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // These operations should never crash, regardless of platform
    EXPECT_NO_THROW({
        outline_mgr.createTOC();
        outline_mgr.getOutlineNodeCount();
        outline_mgr.getBodyNodeName(0);
        outline_mgr.getBodyNodeName(999); // Out of bounds
        outline_mgr.clear();
    });
    
    // Generate outline should also be safe
    EXPECT_NO_THROW({
        auto result = outline_mgr.generate_outline_safe();
        // Result might be error or success, but should not crash
    });
}

TEST_F(UnifiedOutlineManagerTest, CrossPlatform_EdgeCases)
{
    auto doc_result = Document::create_safe(get_test_output_path("unified_outline_edge_cases.docx"));
    ASSERT_TRUE(doc_result.ok()) << doc_result.error().message();
    
    auto& doc = doc_result.value();
    auto& outline_mgr = doc.outline();
    
    // Empty manager
    EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 0);
    EXPECT_TRUE(outline_mgr.getBodyNodeName(0).empty());
    
    // Large indices
    std::vector<size_t> large_indices = {SIZE_MAX, SIZE_MAX - 1, 1000000};
    for (size_t idx : large_indices) {
        std::string name = outline_mgr.getBodyNodeName(idx);
        EXPECT_TRUE(name.empty()) << "Large index " << idx << " should return empty";
    }
    
    // Multiple operations should be safe
    for (int i = 0; i < 10; ++i) {
        outline_mgr.createTOC();
        EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 5);
        outline_mgr.clear();
        EXPECT_EQ(outline_mgr.getOutlineNodeCount(), 0);
    }
}