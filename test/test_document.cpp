#include <array>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Document.hpp"
#include "Body.hpp"
#include "BaseElement.hpp"
#include <string>
#include <stdexcept>

// Mock classes for dependencies that are not the focus of this test
// We assume these are tested elsewhere and provide minimal implementations or mocks.
// For this test, we will perform more of an integration test since Document's
// core functionality is tied to file operations.

class DocumentTest : public ::testing::Test {
protected:
    std::string test_docx_path;
    std::string another_test_docx_path;

    void SetUp() override {
        test_docx_path = "test_document.docx";
        another_test_docx_path = "another_test_document.docx";
    }

    void TearDown() override {
        // Clean up created files
        remove(test_docx_path.c_str());
        remove(another_test_docx_path.c_str());
    }
};

// Test static factory methods
TEST_F(DocumentTest, Create_CreatesFileOnSave) {
    ASSERT_NO_THROW({
        const auto doc = duckx::Document::create(test_docx_path);
        doc.save();
    });

    // Check if the file now exists
    FILE* file = fopen(test_docx_path.c_str(), "r");
    ASSERT_NE(file, nullptr);
    if (file) {
        fclose(file);
    }
}

TEST_F(DocumentTest, Open_ValidFile) {
    {
        const auto doc = duckx::Document::create(test_docx_path);
        doc.save();
    }

    ASSERT_NO_THROW({
        auto doc = duckx::Document::open(test_docx_path);
    });
}

TEST_F(DocumentTest, Open_NonExistentFile) {
    EXPECT_THROW(duckx::Document::open("non_existent_file.docx"), std::runtime_error);
}

// Test move semantics
TEST_F(DocumentTest, MoveConstructor) {
    auto doc1 = duckx::Document::create(test_docx_path);
    doc1.body().add_paragraph("Move test");

    // Move doc1 to doc2
    const duckx::Document doc2 = std::move(doc1);

    // doc2 should be functional
    ASSERT_NO_THROW(doc2.save());

    // Verify content through doc2
    auto reopened_doc = duckx::Document::open(test_docx_path);
    auto paragraphs = reopened_doc.body().paragraphs();
    auto it = paragraphs.begin();
    ASSERT_NE(it, paragraphs.end());

    auto runs = it->runs();
    auto run_it = runs.begin();
    ASSERT_NE(run_it, runs.end());
    EXPECT_EQ(run_it->get_text(), "Move test");
}

TEST_F(DocumentTest, MoveAssignmentOperator) {
    auto doc1 = duckx::Document::create(test_docx_path);
    doc1.body().add_paragraph("Original doc");

    auto doc2 = duckx::Document::create(another_test_docx_path);
    doc2.body().add_paragraph("Target doc");

    // Move assign doc1 to doc2
    doc2 = std::move(doc1);

    // doc2 should now point to doc1's data and file
    ASSERT_NO_THROW(doc2.save());

    // Open the original file path and check for "Original doc"
    auto reopened_doc = duckx::Document::open(test_docx_path);
    auto paragraphs = reopened_doc.body().paragraphs();
    auto it = paragraphs.begin();
    ASSERT_NE(it, paragraphs.end());

    auto runs = it->runs();
    auto run_it = runs.begin();
    ASSERT_NE(run_it, runs.end());
    EXPECT_EQ(run_it->get_text(), "Original doc");

    // The other file should not have been saved by doc2
    auto another_doc = duckx::Document::open(another_test_docx_path);
    auto another_paragraphs = another_doc.body().paragraphs();
    auto another_it = another_paragraphs.begin();
    auto run_text = another_it->runs().begin()->get_text();
    EXPECT_NE(run_text, "Original doc");
}

// Test core functionalities
TEST_F(DocumentTest, Save) {
    auto doc = duckx::Document::create(test_docx_path);
    doc.body().add_paragraph("Testing save functionality.");

    ASSERT_NO_THROW(doc.save());

    // Re-open and verify content
    auto reopened_doc = duckx::Document::open(test_docx_path);
    auto paragraphs = reopened_doc.body().paragraphs();
    auto it = paragraphs.begin();

    ASSERT_NE(it, paragraphs.end());
    EXPECT_EQ(it->runs().begin()->get_text(), "Testing save functionality.");
}

TEST_F(DocumentTest, BodyAccessor) {
    auto doc = duckx::Document::create(test_docx_path);

    // Non-const version
    duckx::Body& body = doc.body();
    body.add_paragraph("Hello");
    ASSERT_EQ(std::distance(doc.body().paragraphs().begin(), doc.body().paragraphs().end()), 1);

    // Const version
    const auto& const_doc = doc;
    const duckx::Body& const_body = const_doc.body();
    ASSERT_EQ(std::distance(const_body.paragraphs().begin(), const_body.paragraphs().end()), 1);
}

TEST_F(DocumentTest, AccessorFunctionality) {
    auto doc = duckx::Document::create(test_docx_path);

    ASSERT_NO_THROW(doc.media());
    ASSERT_NO_THROW(doc.links());

    // These should not throw even if headers/footers don't exist yet
    ASSERT_NO_THROW(doc.get_header(duckx::HeaderFooterType::DEFAULT));
    ASSERT_NO_THROW(doc.get_footer(duckx::HeaderFooterType::DEFAULT));
    ASSERT_NO_THROW(doc.get_header(duckx::HeaderFooterType::FIRST));
    ASSERT_NO_THROW(doc.get_footer(duckx::HeaderFooterType::FIRST));
    ASSERT_NO_THROW(doc.get_header(duckx::HeaderFooterType::EVEN));
    ASSERT_NO_THROW(doc.get_footer(duckx::HeaderFooterType::EVEN));
}

TEST_F(DocumentTest, GetNextRelationshipIdWithPresetId) {
    constexpr int test_count = 3;
    std::array<std::string, test_count> expected_ids;
    for (int i = 0; i < test_count; ++i)
    {
        expected_ids[i] = "rId" + std::to_string(duckx::preset_id + i + 1);
    }

    auto doc = duckx::Document::create(test_docx_path);

    for (int i = 0; i < test_count; ++i) {
        std::string rid = doc.get_next_relationship_id();
        EXPECT_EQ(rid, expected_ids[i]);
    }
}

TEST_F(DocumentTest, GetNextRelationshipId) {
        auto doc = duckx::Document::create(test_docx_path);

        const std::string rId1 = doc.get_next_relationship_id();
        const std::string rId2 = doc.get_next_relationship_id();
        const std::string rId3 = doc.get_next_relationship_id();

        EXPECT_TRUE(rId1.substr(0, 3) == "rId");
        EXPECT_TRUE(rId2.substr(0, 3) == "rId");
        EXPECT_TRUE(rId3.substr(0, 3) == "rId");

        const int id1_num = std::stoi(rId1.substr(3));
        const int id2_num = std::stoi(rId2.substr(3));
        const int id3_num = std::stoi(rId3.substr(3));

        EXPECT_EQ(id2_num, id1_num + 1);
        EXPECT_EQ(id3_num, id2_num + 1);

        // 确保每个ID都是唯一的
        EXPECT_NE(rId1, rId2);
        EXPECT_NE(rId2, rId3);
        EXPECT_NE(rId1, rId3);
    }

TEST_F(DocumentTest, GetUniqueRid) {
    auto doc = duckx::Document::create(test_docx_path);

    const unsigned int id1 = doc.get_unique_rid();
    const unsigned int id2 = doc.get_unique_rid();
    const unsigned int id3 = doc.get_unique_rid();

    EXPECT_GT(id2, id1);
    EXPECT_GT(id3, id2);
}

// Integration-style test for combined operations
TEST_F(DocumentTest, AddContentAndSaveAndReopen) {
    {
        auto doc = duckx::Document::create(test_docx_path);
        duckx::Body& body = doc.body();

        body.add_paragraph("This is the first paragraph.");
        body.add_paragraph("This is the second, with bold.", duckx::bold);
        body.add_table(3, 4);

        doc.save();
    }

    {
        auto doc = duckx::Document::open(test_docx_path);
        const duckx::Body& body = doc.body();

        // Verify paragraphs
        auto paragraphs = body.paragraphs();
        ASSERT_EQ(std::distance(paragraphs.begin(), paragraphs.end()), 2);

        auto p_it = paragraphs.begin();
        EXPECT_EQ(p_it->runs().begin()->get_text(), "This is the first paragraph.");

        ++p_it;
        auto r_it = p_it->runs().begin();
        EXPECT_EQ(r_it->get_text(), "This is the second, with bold.");
        // We cannot easily verify formatting without a deeper inspection of XML,
        // but the call should succeed.

        // Verify tables
        auto tables = body.tables();
        ASSERT_EQ(std::distance(tables.begin(), tables.end()), 1);

        auto t_it = tables.begin();
        // Here we'd need Table API to check rows/cols, which is beyond Document's scope
        // Just checking for existence is sufficient for this test.
    }
}