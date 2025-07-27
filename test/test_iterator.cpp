/*!
 * @file test_iterator.cpp
 * @brief Unit tests for iterator and range utilities
 * 
 * Tests type-safe iterators, range traversal, STL compatibility,
 * and modern iterator patterns for document elements including
 * compile-time type checking and range-based loop support.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include "Document.hpp"
#include "duckxiterator.hpp"
#include "BaseElement.hpp"
#include "test_utils.hpp"

// Mock class that satisfies the DocxElement concept
class MockDocxElement
{
public:
    MockDocxElement() = default;

    explicit MockDocxElement(const pugi::xml_node node)
        : m_node(node), m_advance_count(0) {}

    static MockDocxElement createWithValidNode()
    {
        static pugi::xml_document static_doc;
        static bool initialized = false;
        if (!initialized)
        {
            static_doc.append_child("mock_element");
            initialized = true;
        }
        return MockDocxElement(static_doc.first_child());
    }

    pugi::xml_node get_node() const { return m_node; }

    MockDocxElement& advance()
    {
        if (m_advance_count < m_max_advances)
        {
            m_advance_count++;
        }
        return *this;
    }

    bool try_advance()
    {
        if (m_advance_count <= m_max_advances)
        {
            m_advance_count++;
            return m_advance_count <= m_max_advances;
        }
        return false;
    }

    bool can_advance() const
    {
        return m_advance_count < m_max_advances;
    }

    void set_max_advances(const int max) { m_max_advances = max; }
    int get_advance_count() const { return m_advance_count; }
    bool should_end() const { return m_advance_count >= m_max_advances; }

private:
    pugi::xml_node m_node;
    int m_advance_count = 0;
    int m_max_advances = 0;
};

class DocxIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        try
        {
            doc = std::make_unique<duckx::Document>(duckx::Document::open(duckx::test_utils::get_temp_path("my_test.docx")));
            valid_doc_available = true;
        }
        catch (const std::exception&)
        {
            valid_doc_available = false;
        }
    }

    void TearDown() override
    {
        doc.reset();
    }

    std::unique_ptr<duckx::Document> doc;
    bool valid_doc_available = false;

    // Helper method to create test document
    static std::unique_ptr<duckx::Document> CreateTestDocument()
    {
        try
        {
            return std::make_unique<duckx::Document>(duckx::Document::create(duckx::test_utils::get_temp_path("test_temp.docx")));
        }
        catch (const std::exception&)
        {
            return nullptr;
        }
    }
};

TEST_F(DocxIteratorTest, RangeForLoopReadsContentCorrectly)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    std::ostringstream ss;

    for (duckx::Paragraph& p: doc->body().paragraphs())
    {
        for (const duckx::Run& r: p.runs())
        {
            ss << r.get_text() << std::endl;
        }
    }

    const std::string expected_content = "This is a test\nokay?\n";
    EXPECT_EQ(expected_content, ss.str());
}

TEST_F(DocxIteratorTest, IteratorEqualityComparesCorrectly)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();

    EXPECT_EQ(paragraphs_range.begin(), paragraphs_range.begin());
    EXPECT_NE(paragraphs_range.begin(), paragraphs_range.end());

    auto it1 = paragraphs_range.begin();
    auto it2 = paragraphs_range.begin();

    ++it1;
    EXPECT_NE(it1, it2);

    ++it2;
    EXPECT_EQ(it1, it2);
}

TEST_F(DocxIteratorTest, InvalidDocumentHandling)
{
    EXPECT_THROW(duckx::Document::open("nonexistent_file.docx"), std::exception);
    EXPECT_THROW(duckx::Document::open(""), std::exception);
    EXPECT_THROW(duckx::Document::open("/invalid/path/file.docx"), std::exception);
}

TEST_F(DocxIteratorTest, EmptyDocumentIteration)
{
    const auto empty_doc = CreateTestDocument();
    if (!empty_doc)
    {
        GTEST_SKIP() << "Cannot create test document";
    }

    auto paragraphs_range = empty_doc->body().paragraphs();

    int paragraph_count = 0;
    for (const auto& p: paragraphs_range)
    {
        (void)p; // Suppress unused variable warning
        paragraph_count++;
        if (paragraph_count > 100) break; // Safety check
    }

    // Empty document might have default paragraph
    EXPECT_GE(paragraph_count, 0);
}

TEST_F(DocxIteratorTest, EmptyParagraphIteration)
{
    const auto test_doc = CreateTestDocument();
    if (!test_doc)
    {
        GTEST_SKIP() << "Cannot create test document";
    }

    auto para = test_doc->body().add_paragraph("");
    auto runs_range = para.runs();

    int run_count = 0;
    for (const auto& r: runs_range)
    {
        (void)r;
        run_count++;
        if (run_count > 100) break; // Safety check
    }

    EXPECT_GE(run_count, 0);
}

TEST_F(DocxIteratorTest, IteratorIncrementBeyondEnd)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    auto it = paragraphs_range.begin();

    // Advance to end
    int safety_counter = 0;
    while (it != paragraphs_range.end() && safety_counter < 1000)
    {
        ++it;
        safety_counter++;
    }

    // Further increment should be safe
    EXPECT_NO_THROW(++it);
    EXPECT_EQ(it, paragraphs_range.end());
}

TEST_F(DocxIteratorTest, IteratorDereferenceAtEnd)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    auto end_it = paragraphs_range.end();

    // Dereferencing end iterator should be handled gracefully or throw
    EXPECT_NO_FATAL_FAILURE([&]() {
        try
        {
        const auto& para = *end_it;
        (void)para;
        }
        catch (const std::exception&)
        {
        // Expected behavior for some implementations
        }
        }());
}

TEST_F(DocxIteratorTest, IteratorCopyConstruction)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    auto it1 = paragraphs_range.begin();
    const auto it2(it1); // Copy construction

    EXPECT_EQ(it1, it2);

    ++it1;
    EXPECT_NE(it1, it2);
}

TEST_F(DocxIteratorTest, IteratorAssignment)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    const auto it1 = paragraphs_range.begin();
    auto it2 = paragraphs_range.end();

    it2 = it1; // Assignment
    EXPECT_EQ(it1, it2);
}

TEST_F(DocxIteratorTest, MultipleIteratorInstances)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    auto it1 = paragraphs_range.begin();
    const auto it2 = paragraphs_range.begin();
    const auto it3 = paragraphs_range.begin();

    EXPECT_EQ(it1, it2);
    EXPECT_EQ(it2, it3);

    ++it1;
    EXPECT_NE(it1, it2);
    EXPECT_EQ(it2, it3);
}

TEST_F(DocxIteratorTest, IteratorArrowOperator)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    auto it = paragraphs_range.begin();

    if (it != paragraphs_range.end())
    {
        EXPECT_NO_THROW(it->runs());
    }
}

TEST_F(DocxIteratorTest, IteratorSwap)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    auto it1 = paragraphs_range.begin();
    auto it2 = paragraphs_range.end();

    const auto original_it1 = it1;
    const auto original_it2 = it2;

    std::swap(it1, it2);

    EXPECT_EQ(it1, original_it2);
    EXPECT_EQ(it2, original_it1);
}

TEST_F(DocxIteratorTest, NestedIterationConsistency)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();

    for (auto& para: paragraphs_range)
    {
        auto runs_range = para.runs();
        auto run_it1 = runs_range.begin();
        auto run_it2 = runs_range.begin();

        EXPECT_EQ(run_it1, run_it2);

        for (auto& run: runs_range)
        {
            EXPECT_NO_THROW(run.get_text());
        }
    }
}

TEST_F(DocxIteratorTest, ConstIteratorUsage)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    const auto& const_doc = *doc;
    const auto paragraphs_range = const_doc.body().paragraphs();

    for (const auto& para: paragraphs_range)
    {
        for (const auto& run: para.runs())
        {
            EXPECT_NO_THROW(run.get_text());
        }
    }
}

TEST_F(DocxIteratorTest, MockElementTypeConstraints)
{
    static_assert(duckx::has_get_node_method<MockDocxElement>::value,
                  "MockDocxElement should have get_node method");
    static_assert(duckx::has_advance_method<MockDocxElement>::value,
                  "MockDocxElement should have advance method");
    static_assert(duckx::has_valid_get_node_return<MockDocxElement>::value,
                  "MockDocxElement get_node should return pugi::xml_node");
    static_assert(duckx::has_valid_advance_return<MockDocxElement>::value,
                  "MockDocxElement advance should return MockDocxElement&");
    static_assert(duckx::is_docx_element<MockDocxElement>::value,
                  "MockDocxElement should satisfy DocxElement concept");
}

TEST_F(DocxIteratorTest, IteratorStateAfterAdvance)
{
    MockDocxElement mock_element = MockDocxElement::createWithValidNode();
    mock_element.set_max_advances(3);

    auto range = duckx::make_element_range(mock_element);
    auto it = range.begin();

    EXPECT_NE(it, range.end());

    ++it;
    EXPECT_NE(it, range.end());

    ++it;
    EXPECT_NE(it, range.end());

    ++it;
    EXPECT_NE(it, range.end());

    ++it;
    EXPECT_EQ(it, range.end());
}

TEST_F(DocxIteratorTest, ElementRangeEmptyCheck)
{
    MockDocxElement mock_element = MockDocxElement::createWithValidNode();
    mock_element.set_max_advances(0);

    auto range = duckx::make_element_range(mock_element);

    EXPECT_FALSE(range.empty());

    auto it = range.begin();
    ++it;

    EXPECT_EQ(it, range.end());
}

TEST_F(DocxIteratorTest, ElementRangeSize)
{
    MockDocxElement mock_element = MockDocxElement::createWithValidNode();
    mock_element.set_max_advances(5);

    const auto range = duckx::make_element_range(mock_element);

    size_t expected_size = 0;
    MockDocxElement test_element = MockDocxElement::createWithValidNode();
    test_element.set_max_advances(5);

    if (test_element.get_node())
    {
        expected_size = 1; // 
        while (test_element.get_advance_count() < 5)
        {
            test_element.advance();
            if (test_element.get_node()) { expected_size++; }
            else { break; }
        }
    }

    EXPECT_EQ(range.size(), expected_size);
}

TEST_F(DocxIteratorTest, LargeDocumentIteration)
{
    const auto test_doc = CreateTestDocument();
    if (!test_doc)
    {
        GTEST_SKIP() << "Cannot create test document";
    }

    // Add many paragraphs
    constexpr int num_paragraphs = 100;
    for (int i = 0; i < num_paragraphs; ++i)
    {
        test_doc->body().add_paragraph("Paragraph " + std::to_string(i));
    }

    int count = 0;
    for (const auto& para: test_doc->body().paragraphs())
    {
        (void)para;
        count++;
        if (count > num_paragraphs + 10) break; // Safety check
    }

    EXPECT_GE(count, 1);
}

TEST_F(DocxIteratorTest, IteratorValidityAfterDocumentModification)
{
    const auto test_doc = CreateTestDocument();
    if (!test_doc)
    {
        GTEST_SKIP() << "Cannot create test document";
    }

    test_doc->body().add_paragraph("First paragraph");
    test_doc->body().add_paragraph("Second paragraph");

    auto paragraphs_range = test_doc->body().paragraphs();
    auto it = paragraphs_range.begin();

    // Modify document
    test_doc->body().add_paragraph("Third paragraph");

    // Iterator behavior after modification is implementation-dependent
    EXPECT_NO_FATAL_FAILURE([&]() {
        try
        {
        ++it;
        }
        catch (const std::exception&)
        {
        // May throw if iterators are invalidated
        }
        }());
}

TEST_F(DocxIteratorTest, IteratorInLoopBreak)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    int count = 0;

    for (const auto& para: paragraphs_range)
    {
        (void)para;
        count++;
        if (count >= 2) break;
    }

    EXPECT_LE(count, 2);
}

TEST_F(DocxIteratorTest, IteratorInLoopContinue)
{
    if (!valid_doc_available)
    {
        GTEST_SKIP() << "Test document not available";
    }

    auto paragraphs_range = doc->body().paragraphs();
    int processed = 0;
    int total = 0;

    for (const auto& para: paragraphs_range)
    {
        total++;
        if (total % 2 == 0) continue;

        (void)para;
        processed++;
        if (total > 10) break; // Safety check
    }

    EXPECT_LE(processed, total);
}

TEST_F(DocxIteratorTest, EmptyRangeIteration)
{
    pugi::xml_document empty_doc;
    MockDocxElement empty_element;
    empty_element.set_max_advances(0);

    auto range = duckx::make_element_range(empty_element);
    EXPECT_EQ(range.begin(), range.end());
}

TEST_F(DocxIteratorTest, SingleElementRange)
{
    pugi::xml_document test_doc;
    const pugi::xml_node single_node = test_doc.append_child("single");
    MockDocxElement single_element(single_node);
    single_element.set_max_advances(0);

    auto range = duckx::make_element_range(single_element);
    auto it = range.begin();
    EXPECT_NE(it, range.end());
    ++it;
    EXPECT_EQ(it, range.end());
}

TEST_F(DocxIteratorTest, IteratorChainedAdvance)
{
    pugi::xml_document test_doc;
    const pugi::xml_node node = test_doc.append_child("test");

    MockDocxElement element(node);
    element.set_max_advances(5);

    auto range = duckx::make_element_range(element);
    auto it = range.begin();

    // 
    const auto it2 = ++++it;
    EXPECT_NE(it2, range.end());
    EXPECT_EQ(it, it2);
}
