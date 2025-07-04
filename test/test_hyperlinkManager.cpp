/*!
 * @brief Test suite for the duckx::HyperlinkManager class.
 *
 * @author LiuYe
 * @date 2025.07.05
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "HyperlinkManager.hpp"
#include "Document.hpp"
#include <string>

// Since Document is complex to fully mock and its rId generation is crucial,
// we'll use a real (but temporary) Document object for testing.
// This makes it more of an integration test for the HyperlinkManager.
class HyperlinkManagerTest : public ::testing::Test
{
protected:
    std::unique_ptr<duckx::Document> m_doc;
    pugi::xml_document m_rels_xml;
    std::unique_ptr<duckx::HyperlinkManager> m_link_manager;

    void SetUp() override
    {
        // Create a temporary, in-memory document to provide context, especially for rId generation.
        // We use a temporary file path that won't actually be saved unless explicitly told to.
        m_doc = std::make_unique<duckx::Document>(duckx::Document::create("temp_doc_for_hyperlink_test.docx"));

        // Prepare a minimal valid rels XML structure.
        m_rels_xml.load_string(
            R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"></Relationships>)");

        // Instantiate the HyperlinkManager with the real document and the test XML.
        m_link_manager = std::make_unique<duckx::HyperlinkManager>(m_doc.get(), &m_rels_xml);
    }

    void TearDown() override
    {
        // Clean up is handled by smart pointers.
    }
};

// ============================================================================
// Constructor Test
// ============================================================================

TEST_F(HyperlinkManagerTest, Constructor_InitializesCorrectly)
{
    // The SetUp method already constructs the manager.
    // This test simply verifies that the construction does not throw an exception.
    SUCCEED();
}

// ============================================================================
// add_relationship Method Tests
// ============================================================================

TEST_F(HyperlinkManagerTest, AddRelationship_AddsSingleValidURL)
{
    const std::string url = "https://www.google.com";
    const std::string rId = m_link_manager->add_relationship(url);

    // 1. Verify the returned relationship ID is not empty and has the correct prefix.
    EXPECT_FALSE(rId.empty());
    EXPECT_THAT(rId, testing::StartsWith("rId"));

    // 2. Verify the XML structure was updated correctly.
    const pugi::xml_node rel_node = m_rels_xml.child("Relationships").find_child_by_attribute("Id", rId.c_str());
    ASSERT_TRUE(rel_node);

    // 3. Verify the attributes of the new Relationship node.
    EXPECT_STREQ(rel_node.attribute("Id").value(), rId.c_str());
    EXPECT_STREQ(rel_node.attribute("Type").value(),
                 "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink");
    EXPECT_STREQ(rel_node.attribute("Target").value(), url.c_str());
    EXPECT_STREQ(rel_node.attribute("TargetMode").value(), "External");
}

TEST_F(HyperlinkManagerTest, AddRelationship_AddsMultipleURLsWithUniqueIds)
{
    const std::string url1 = "https://github.com";
    const std::string url2 = "https://www.microsoft.com";

    const std::string rId1 = m_link_manager->add_relationship(url1);
    const std::string rId2 = m_link_manager->add_relationship(url2);

    // 1. Verify that the relationship IDs are unique.
    EXPECT_NE(rId1, rId2);

    // 2. Verify both relationships exist in the XML.
    const pugi::xml_node rels_root = m_rels_xml.child("Relationships");
    const pugi::xml_node rel_node1 = rels_root.find_child_by_attribute("Id", rId1.c_str());
    const pugi::xml_node rel_node2 = rels_root.find_child_by_attribute("Id", rId2.c_str());

    ASSERT_TRUE(rel_node1);
    ASSERT_TRUE(rel_node2);

    // 3. Verify the targets are correct.
    EXPECT_STREQ(rel_node1.attribute("Target").value(), url1.c_str());
    EXPECT_STREQ(rel_node2.attribute("Target").value(), url2.c_str());

    // 4. Verify there are exactly two relationships.
    EXPECT_EQ(std::distance(rels_root.children("Relationship").begin(), rels_root.children("Relationship").end()), 2);
}

TEST_F(HyperlinkManagerTest, AddRelationship_HandlesEmptyURL)
{
    const std::string url;
    const std::string rId = m_link_manager->add_relationship(url);

    // An empty URL should still create a valid relationship entry.
    EXPECT_FALSE(rId.empty());

    const pugi::xml_node rel_node = m_rels_xml.child("Relationships").find_child_by_attribute("Id", rId.c_str());
    ASSERT_TRUE(rel_node);

    // The Target attribute should be present but empty.
    EXPECT_STREQ(rel_node.attribute("Target").value(), "");
}

TEST_F(HyperlinkManagerTest, AddRelationship_HandlesURLWithSpecialXMLCharacters)
{
    // URLs can contain characters that need to be handled correctly in XML attributes.
    // PugiXML handles this automatically, this test verifies it.
    const std::string url_with_ampersand = "https://example.com/search?q=C++&version=20";
    const std::string rId = m_link_manager->add_relationship(url_with_ampersand);

    const pugi::xml_node rel_node = m_rels_xml.child("Relationships").find_child_by_attribute("Id", rId.c_str());
    ASSERT_TRUE(rel_node);

    // The attribute value should be the original string, pugi::xml handles the escaping (`&` to `&amp;`) during serialization.
    EXPECT_STREQ(rel_node.attribute("Target").value(), url_with_ampersand.c_str());
}
