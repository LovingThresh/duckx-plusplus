/*!
 * @file test_docxFile.cpp
 * @brief Unit tests for DocxFile class and ZIP archive operations
 * 
 * Tests low-level DOCX file operations including ZIP archive management,
 * entry reading/writing, file creation, and XML template generation.
 * Validates underlying file system operations for document management.
 * 
 * @date 2025.07
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "DocxFile.hpp"
#include <fstream>
#include <string>
#include <cstdio>  // For remove()

#if defined(_WIN32)
#include <direct.h> // For _mkdir
#else
#include <sys/stat.h> // For mkdir
#include <sys/types.h>
#endif

// Since zip_t is an opaque pointer from an external C library (libzip),
// we will perform integration-style tests that interact with the filesystem.
// These tests require a real zip library to be linked for these tests to compile and run.

class DocxFileTest : public ::testing::Test
{
protected:
    std::string test_dir;

    void SetUp() override
    {
        test_dir = "test_temp_dir";
#if defined(_WIN32)
        _mkdir(test_dir.c_str());
#else
        mkdir(test_dir.c_str(), 0777);
#endif
    }

    void TearDown() override
    {
        // Using a system call is the easiest way to recursively delete the directory.
#if defined(_WIN32)
        std::string command = "rd /s /q " + test_dir;
#else
        std::string command = "rm -rf " + test_dir;
#endif
        system(command.c_str());
    }

    std::string get_test_path(const std::string& file_name) const
    {
        return test_dir + "/" + file_name;
    }

    static bool file_exists(const std::string& name)
    {
        const std::ifstream f(name.c_str());
        return f.good();
    }
};

TEST_F(DocxFileTest, DefaultConstructor)
{
    const duckx::DocxFile doc;
    EXPECT_TRUE(doc.m_path.empty());
    EXPECT_EQ(doc.m_zip_handle, nullptr); // 在按需打开设计中，这应该始终为nullptr
    EXPECT_TRUE(doc.m_dirty_entries.empty());
}

TEST_F(DocxFileTest, CreateNewFileSuccess)
{
    duckx::DocxFile doc;
    const std::string path = get_test_path("new_doc.docx");

    ASSERT_TRUE(doc.create(path));

    EXPECT_EQ(doc.m_path, path);
    EXPECT_EQ(doc.m_zip_handle, nullptr); // 按需打开设计：create后不保持文件打开

    doc.save();
    doc.close();

    EXPECT_TRUE(file_exists(path));
}

TEST_F(DocxFileTest, CreateFileInInvalidPathFails)
{
    duckx::DocxFile doc;
    const std::string path = "non_existent_dir/new_doc.docx";
    EXPECT_FALSE(doc.create(path));
    EXPECT_EQ(doc.m_zip_handle, nullptr);
}

TEST_F(DocxFileTest, OpenExistingFileSuccess)
{
    const std::string path = get_test_path("existing.docx");

    // First, create a file to open.
    {
        duckx::DocxFile creator;
        ASSERT_TRUE(creator.create(path));
        creator.save();
        creator.close();
    }

    // Now, open it.
    duckx::DocxFile reader;
    EXPECT_TRUE(reader.open(path));
    EXPECT_EQ(reader.m_path, path);
    EXPECT_EQ(reader.m_zip_handle, nullptr); // 按需打开设计：open后不保持文件打开

    // 验证功能：应该能正常访问文件内容
    EXPECT_TRUE(reader.has_entry("word/document.xml"));
    EXPECT_TRUE(reader.has_entry("[Content_Types].xml"));

    reader.close();
}

TEST_F(DocxFileTest, OpenNonExistentFileFails)
{
    duckx::DocxFile doc;
    const std::string path = get_test_path("non_existent.docx");
    EXPECT_FALSE(doc.open(path));
    EXPECT_EQ(doc.m_zip_handle, nullptr);
}

TEST_F(DocxFileTest, OpenInvalidZipFileFails)
{
    const std::string path = get_test_path("not_a_zip.txt");

    // Create a plain text file.
    std::ofstream ofs(path.c_str());
    ofs << "This is not a zip file.";
    ofs.close();

    duckx::DocxFile doc;
    EXPECT_FALSE(doc.open(path));
    EXPECT_EQ(doc.m_zip_handle, nullptr);
}

TEST_F(DocxFileTest, CloseResetsState)
{
    duckx::DocxFile doc;
    const std::string path = get_test_path("to_close.docx");

    ASSERT_TRUE(doc.create(path));
    EXPECT_EQ(doc.m_zip_handle, nullptr); // 按需打开设计：create后句柄为nullptr

    doc.close();
    EXPECT_EQ(doc.m_zip_handle, nullptr);
    EXPECT_TRUE(doc.m_path.empty());
    EXPECT_TRUE(doc.m_dirty_entries.empty());
}

TEST_F(DocxFileTest, WriteAndReadEntryInMemory)
{
    duckx::DocxFile doc;
    const std::string path = get_test_path("write_read_mem.docx");
    ASSERT_TRUE(doc.create(path));

    const std::string entry_name = "custom/data.txt";
    const std::string content = "This is some custom data.";

    doc.write_entry(entry_name, content);

    EXPECT_TRUE(doc.has_entry(entry_name));
    EXPECT_EQ(doc.read_entry(entry_name), content);

    // Check the dirty entries map.
    EXPECT_EQ(doc.m_dirty_entries.count(entry_name), 1);
    EXPECT_EQ(doc.m_dirty_entries[entry_name], content);

    doc.close();
}

TEST_F(DocxFileTest, WriteAndSaveAndReopen)
{
    const std::string path = get_test_path("save_and_reopen.docx");
    const std::string entry_name = "word/custom.xml";
    const std::string content = "<custom>Saved Content</custom>";

    // Create, write, save, close.
    {
        duckx::DocxFile writer;
        ASSERT_TRUE(writer.create(path));
        writer.write_entry(entry_name, content);
        writer.save();
        writer.close();
    }

    // Reopen and read.
    {
        duckx::DocxFile reader;
        ASSERT_TRUE(reader.open(path));
        EXPECT_TRUE(reader.has_entry(entry_name));
        EXPECT_EQ(reader.read_entry(entry_name), content);
        reader.close();
    }
}

TEST_F(DocxFileTest, HasEntryForDefaultAndCustomEntries)
{
    duckx::DocxFile doc;
    const std::string path = get_test_path("has_entry.docx");
    ASSERT_TRUE(doc.create(path));

    // Check for default entries created by create_basic_structure.
    EXPECT_TRUE(doc.has_entry("[Content_Types].xml"));
    EXPECT_TRUE(doc.has_entry("docProps/core.xml"));
    EXPECT_TRUE(doc.has_entry("word/document.xml"));

    // Check for non-existent entry.
    EXPECT_FALSE(doc.has_entry("non/existent/entry.txt"));

    // Check after adding a dirty entry.
    doc.write_entry("custom/dirty.txt", "dirty data");
    EXPECT_TRUE(doc.has_entry("custom/dirty.txt"));

    doc.save();

    // Should still be true after save.
    EXPECT_TRUE(doc.has_entry("custom/dirty.txt"));

    doc.close();
}

TEST_F(DocxFileTest, ReadNonExistentEntry)
{
    duckx::DocxFile doc;
    const std::string path = get_test_path("read_non_existent.docx");
    ASSERT_TRUE(doc.create(path));

    // 根据实现，读取不存在的条目应该抛出异常或返回空字符串
    // 从实现看，对于不存在的条目会抛出异常，除非是word/document.xml会返回默认内容
    EXPECT_THROW(doc.read_entry("non/existent/entry.txt"), std::runtime_error);

    doc.close();
}

TEST_F(DocxFileTest, ModifyExistingEntry)
{
    std::string path = get_test_path("modify.docx");
    std::string original_content = duckx::DocxFile::get_app_xml();
    std::string modified_content = "<app>Modified</app>";

    // Create and save.
    {
        duckx::DocxFile writer;
        ASSERT_TRUE(writer.create(path));
        ASSERT_EQ(writer.read_entry("docProps/app.xml"), original_content);
        writer.save();
        writer.close();
    }

    // Reopen, modify, save.
    {
        duckx::DocxFile modifier;
        ASSERT_TRUE(modifier.open(path));
        modifier.write_entry("docProps/app.xml", modified_content);
        modifier.save();
        modifier.close();
    }

    // Verify the modification.
    {
        duckx::DocxFile reader;
        ASSERT_TRUE(reader.open(path));
        EXPECT_EQ(reader.read_entry("docProps/app.xml"), modified_content);
        reader.close();
    }
}

TEST_F(DocxFileTest, StaticXMLGetters)
{
    EXPECT_FALSE(duckx::DocxFile::get_content_types_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_content_types_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_app_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_app_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_core_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_core_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_rels_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_rels_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_document_rels_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_document_rels_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_empty_document_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_empty_document_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_styles_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_styles_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_settings_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_settings_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_font_table_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_font_table_xml(), testing::StartsWith("<?xml"));

    EXPECT_FALSE(duckx::DocxFile::get_default_numbering_xml().empty());
    EXPECT_THAT(duckx::DocxFile::get_default_numbering_xml(), testing::StartsWith("<?xml"));
}

TEST_F(DocxFileTest, SaveWithNoChanges)
{
    std::string path = get_test_path("no_changes.docx");

    // Create a file.
    {
        duckx::DocxFile writer;
        ASSERT_TRUE(writer.create(path));
        writer.save();
        writer.close();
    }

    ASSERT_TRUE(file_exists(path));

    // Reopen and save with no changes.
    {
        duckx::DocxFile reader;
        ASSERT_TRUE(reader.open(path));
        reader.save(); // Should not do anything as no entries are dirty.
        reader.close();
    }

    // The key is that the file is not corrupted and still exists.
    EXPECT_TRUE(file_exists(path));
    // Reopen to ensure it's still valid.
    duckx::DocxFile verifier;
    EXPECT_TRUE(verifier.open(path));
    EXPECT_TRUE(verifier.has_entry("word/document.xml"));
    verifier.close();
}
