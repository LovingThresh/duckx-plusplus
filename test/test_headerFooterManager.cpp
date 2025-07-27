/*!
 * @file test_headerFooterManager.cpp
 * @brief Unit tests for HeaderFooterManager class functionality
 * 
 * Tests header and footer creation, management, and persistence
 * with support for different header/footer types, XML serialization,
 * and comprehensive document structure integration.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Document.hpp"
#include "DocxFile.hpp"
#include "HeaderFooterBase.hpp"
#include "constants.hpp"

#include <cstdio>
#include <string>
#include <memory>
#include <vector>

// Helper function to read the target file from a relationship XML and a relationship ID.
std::string getTargetFromRels(const std::string& relsXmlContent, const std::string& rId)
{
    pugi::xml_document relsDoc;
    if (!relsDoc.load_string(relsXmlContent.c_str())) return "";

    const pugi::xml_node relNode = relsDoc.child("Relationships").find_child_by_attribute(
        "Relationship", "Id", rId.c_str());
    if (relNode)
    {
        return relNode.attribute("Target").as_string();
    }
    return "";
}

class HeaderFooterTest : public ::testing::Test
{
protected:
    const std::string test_filename = "header_footer_test.docx";

    void SetUp() override
    {
        // Ensure no old file exists
        remove(test_filename.c_str());
    }

    void TearDown() override
    {
        // Clean up the created file
        remove(test_filename.c_str());
    }

    // Helper to check for header/footer reference in document.xml
    static bool verify_hf_reference(const std::string& docXmlContent, const std::string& refType,
                                    const std::string& hfType,
                                    std::string& outRId)
    {
        pugi::xml_document doc;
        if (!doc.load_string(docXmlContent.c_str())) return false;

        const auto sectPr = doc.child("w:document").child("w:body").child("w:sectPr");
        if (!sectPr) return false;

        for (const auto& child: sectPr.children())
        {
            if (std::string(child.name()) == refType && std::string(child.attribute("w:type").as_string()) == hfType)
            {
                outRId = child.attribute("r:id").as_string();
                return true;
            }
        }
        return false;
    }
};

// --- HEADER TESTS ---

TEST_F(HeaderFooterTest, GetDefaultHeaderAndAddParagraph)
{ {
        auto doc = duckx::Document::create(test_filename);
        auto& header = doc.get_header(duckx::HeaderFooterType::DEFAULT);
        header.add_paragraph("This is the default header.");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));

    std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "default", rId));
    ASSERT_FALSE(rId.empty());

    std::string doc_rels_content = file.read_entry("word/_rels/document.xml.rels");
    std::string header_filename = getTargetFromRels(doc_rels_content, rId);
    ASSERT_FALSE(header_filename.empty());

    ASSERT_TRUE(file.has_entry("word/" + header_filename));
    std::string header_content = file.read_entry("word/" + header_filename);

    pugi::xml_document header_doc;
    ASSERT_TRUE(header_doc.load_string(header_content.c_str()));
    std::string text = header_doc.child("w:hdr").child("w:p").child("w:r").child("w:t").text().as_string();
    EXPECT_EQ(text, "This is the default header.");
}

TEST_F(HeaderFooterTest, GetFirstPageHeaderAndAddParagraph)
{ {
        const auto doc = duckx::Document::create(test_filename);
        auto& header = doc.get_header(duckx::HeaderFooterType::FIRST);
        header.add_paragraph("This is the first page header.");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));
    const std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "first", rId));

    const std::string header_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);
    const std::string header_content = file.read_entry("word/" + header_filename);

    pugi::xml_document header_doc;
    ASSERT_TRUE(header_doc.load_string(header_content.c_str()));
    const std::string text = header_doc.child("w:hdr").child("w:p").child("w:r").child("w:t").text().as_string();
    EXPECT_EQ(text, "This is the first page header.");
}

TEST_F(HeaderFooterTest, GetEvenPageHeaderAndAddParagraph)
{ {
        const auto doc = duckx::Document::create(test_filename);
        auto& header = doc.get_header(duckx::HeaderFooterType::EVEN);
        header.add_paragraph("This is an even page header.");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));
    const std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "even", rId));

    const std::string header_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);
    const std::string header_content = file.read_entry("word/" + header_filename);

    pugi::xml_document header_doc;
    ASSERT_TRUE(header_doc.load_string(header_content.c_str()));
    const std::string text = header_doc.child("w:hdr").child("w:p").child("w:r").child("w:t").text().as_string();
    EXPECT_EQ(text, "This is an even page header.");
}

TEST_F(HeaderFooterTest, AddAllHeaderTypes)
{ {
        auto doc = duckx::Document::create(test_filename);
        doc.get_header(duckx::HeaderFooterType::DEFAULT).add_paragraph("Default");
        doc.get_header(duckx::HeaderFooterType::FIRST).add_paragraph("First");
        doc.get_header(duckx::HeaderFooterType::EVEN).add_paragraph("Even");
        doc.get_header(duckx::HeaderFooterType::ODD).add_paragraph("Odd");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));
    std::string doc_xml_content = file.read_entry("word/document.xml");

    std::string rId_default, rId_first, rId_even, rId_odd;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "default", rId_default));
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "first", rId_first));
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "even", rId_even));
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "odd", rId_odd));

    ASSERT_NE(rId_default, rId_first);
    ASSERT_NE(rId_default, rId_even);
    ASSERT_NE(rId_first, rId_even);
}

TEST_F(HeaderFooterTest, AddTableToHeader)
{ {
        auto doc = duckx::Document::create(test_filename);
        auto& header = doc.get_header(duckx::HeaderFooterType::DEFAULT);
        header.add_table(3, 4);
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));

    std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "default", rId));
    std::string header_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);

    std::string header_content = file.read_entry("word/" + header_filename);
    pugi::xml_document header_doc;
    ASSERT_TRUE(header_doc.load_string(header_content.c_str()));

    auto table_node = header_doc.child("w:hdr").child("w:tbl");
    ASSERT_TRUE(table_node);

    auto row_count = std::distance(table_node.children("w:tr").begin(), table_node.children("w:tr").end());
    EXPECT_EQ(row_count, 3);

    auto first_row = table_node.child("w:tr");
    ASSERT_TRUE(first_row);
    auto cell_count = std::distance(first_row.children("w:tc").begin(), first_row.children("w:tc").end());
    EXPECT_EQ(cell_count, 4);
}

TEST_F(HeaderFooterTest, IdempotentHeaderAccess)
{
    auto doc = duckx::Document::create(test_filename);
    auto& header1 = doc.get_header(duckx::HeaderFooterType::DEFAULT);
    header1.add_paragraph("First access.");

    auto& header2 = doc.get_header(duckx::HeaderFooterType::DEFAULT);
    header2.add_paragraph("Second access.");

    // Check they are the same instance
    ASSERT_EQ(&header1, &header2);

    doc.save();

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));

    std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:headerReference", "default", rId));
    std::string header_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);

    std::string header_content = file.read_entry("word/" + header_filename);
    pugi::xml_document header_doc;
    ASSERT_TRUE(header_doc.load_string(header_content.c_str()));

    auto paragraphs = header_doc.child("w:hdr").children("w:p");
    auto p_it = paragraphs.begin();
    ASSERT_NE(p_it, paragraphs.end());
    EXPECT_EQ(std::string(p_it->child("w:r").child("w:t").text().as_string()), "First access.");

    ++p_it;
    ASSERT_NE(p_it, paragraphs.end());
    EXPECT_EQ(std::string(p_it->child("w:r").child("w:t").text().as_string()), "Second access.");
}

// --- FOOTER TESTS ---

TEST_F(HeaderFooterTest, GetDefaultFooterAndAddParagraph)
{ {
        auto doc = duckx::Document::create(test_filename);
        auto& footer = doc.get_footer(duckx::HeaderFooterType::DEFAULT);
        footer.add_paragraph("This is the default footer.");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));

    std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "default", rId));
    ASSERT_FALSE(rId.empty());

    std::string doc_rels_content = file.read_entry("word/_rels/document.xml.rels");
    std::string footer_filename = getTargetFromRels(doc_rels_content, rId);
    ASSERT_FALSE(footer_filename.empty());

    ASSERT_TRUE(file.has_entry("word/" + footer_filename));
    std::string footer_content = file.read_entry("word/" + footer_filename);

    pugi::xml_document footer_doc;
    ASSERT_TRUE(footer_doc.load_string(footer_content.c_str()));
    std::string text = footer_doc.child("w:ftr").child("w:p").child("w:r").child("w:t").text().as_string();
    EXPECT_EQ(text, "This is the default footer.");
}

TEST_F(HeaderFooterTest, GetOddPageFooterAndAddParagraph)
{ {
        const auto doc = duckx::Document::create(test_filename);
        auto& footer = doc.get_footer(duckx::HeaderFooterType::ODD);
        footer.add_paragraph("This is an odd page footer.");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));
    const std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "odd", rId));

    const std::string footer_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);
    const std::string footer_content = file.read_entry("word/" + footer_filename);

    pugi::xml_document footer_doc;
    ASSERT_TRUE(footer_doc.load_string(footer_content.c_str()));
    const std::string text = footer_doc.child("w:ftr").child("w:p").child("w:r").child("w:t").text().as_string();
    EXPECT_EQ(text, "This is an odd page footer.");
}

TEST_F(HeaderFooterTest, AddAllFooterTypes)
{ {
        auto doc = duckx::Document::create(test_filename);
        doc.get_footer(duckx::HeaderFooterType::DEFAULT).add_paragraph("Default");
        doc.get_footer(duckx::HeaderFooterType::FIRST).add_paragraph("First");
        doc.get_footer(duckx::HeaderFooterType::EVEN).add_paragraph("Even");
        doc.get_footer(duckx::HeaderFooterType::ODD).add_paragraph("Odd");
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));
    std::string doc_xml_content = file.read_entry("word/document.xml");

    std::string rId_default, rId_first, rId_even, rId_odd;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "default", rId_default));
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "first", rId_first));
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "even", rId_even));
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "odd", rId_odd));

    ASSERT_NE(rId_default, rId_first);
    ASSERT_NE(rId_default, rId_even);
    ASSERT_NE(rId_first, rId_even);
}

TEST_F(HeaderFooterTest, AddTableToFooter)
{ {
        auto doc = duckx::Document::create(test_filename);
        auto& footer = doc.get_footer(duckx::HeaderFooterType::DEFAULT);
        footer.add_table(5, 2);
        doc.save();
    }

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));

    std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "default", rId));
    std::string footer_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);

    std::string footer_content = file.read_entry("word/" + footer_filename);
    pugi::xml_document footer_doc;
    ASSERT_TRUE(footer_doc.load_string(footer_content.c_str()));

    auto table_node = footer_doc.child("w:ftr").child("w:tbl");
    ASSERT_TRUE(table_node);

    auto row_count = std::distance(table_node.children("w:tr").begin(), table_node.children("w:tr").end());
    EXPECT_EQ(row_count, 5);

    auto first_row = table_node.child("w:tr");
    ASSERT_TRUE(first_row);
    auto cell_count = std::distance(first_row.children("w:tc").begin(), first_row.children("w:tc").end());
    EXPECT_EQ(cell_count, 2);
}

TEST_F(HeaderFooterTest, IdempotentFooterAccess)
{
    auto doc = duckx::Document::create(test_filename);
    auto& footer1 = doc.get_footer(duckx::HeaderFooterType::FIRST);
    footer1.add_paragraph("First footer access.");

    auto& footer2 = doc.get_footer(duckx::HeaderFooterType::FIRST);
    footer2.add_paragraph("Second footer access.");

    ASSERT_EQ(&footer1, &footer2);

    doc.save();

    duckx::DocxFile file;
    ASSERT_TRUE(file.open(test_filename));

    std::string doc_xml_content = file.read_entry("word/document.xml");
    std::string rId;
    ASSERT_TRUE(verify_hf_reference(doc_xml_content, "w:footerReference", "first", rId));
    std::string footer_filename = getTargetFromRels(file.read_entry("word/_rels/document.xml.rels"), rId);

    std::string footer_content = file.read_entry("word/" + footer_filename);
    pugi::xml_document footer_doc;
    ASSERT_TRUE(footer_doc.load_string(footer_content.c_str()));

    auto paragraphs = footer_doc.child("w:ftr").children("w:p");
    auto p_it = paragraphs.begin();
    ASSERT_NE(p_it, paragraphs.end());
    EXPECT_EQ(std::string(p_it->child("w:r").child("w:t").text().as_string()), "First footer access.");

    ++p_it;
    ASSERT_NE(p_it, paragraphs.end());
    EXPECT_EQ(std::string(p_it->child("w:r").child("w:t").text().as_string()), "Second footer access.");
}
