/*
 * @file: Document.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "Document.hpp"

#include <iostream>
#include "zip.h"


// Hack on pugixml
// We need to write xml to std string (or char *)
// So overload the write function
struct xml_string_writer : pugi::xml_writer
{
    std::string result;

    // ReSharper disable once CppParameterMayBeConst
    void write(const void* data, size_t size) override
    {
        result.append(static_cast<const char*>(data), size);
    }
};

duckx::Document::Document()
{
    this->m_sDirectory = "";
}

duckx::Document::Document(std::string directory)
{
    this->m_sDirectory = std::move(directory);
}

bool duckx::Document::create()
{
    if (this->m_sDirectory.empty())
    {
        return false;
    }
    return this->create(this->m_sDirectory);
}

bool duckx::Document::create(const std::string& filename)
{
    this->m_sDirectory = filename;

    // 创建新的zip文件
    zip_t* zip = zip_open(filename.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    if (!zip)
    {
        this->m_bFlag_is_open = false;
        return false;
    }

    // 创建docx的基本结构
    create_basic_structure(zip);

    // 关闭zip文件
    zip_close(zip);

    // 设置空的document结构
    this->m_xmlDocument.load_string(get_empty_document_xml().c_str());
    this->m_bFlag_is_open = true;

    // 初始化paragraph
    this->m_paragraph.set_parent(m_xmlDocument.child("w:document").child("w:body"));

    return true;
}

void duckx::Document::create_basic_structure(zip_t* zip)
{
    // 1. [Content_Types].xml
    zip_entry_open(zip, "[Content_Types].xml");
    const std::string content_types = get_content_types_xml();
    zip_entry_write(zip, content_types.c_str(), content_types.length());
    zip_entry_close(zip);

    // 2. _rels/.rels
    zip_entry_open(zip, "_rels/.rels");
    const std::string rels = get_rels_xml();
    zip_entry_write(zip, rels.c_str(), rels.length());
    zip_entry_close(zip);

    // 3. docProps/app.xml
    zip_entry_open(zip, "docProps/app.xml");
    const std::string app = get_app_xml();
    zip_entry_write(zip, app.c_str(), app.length());
    zip_entry_close(zip);

    // 4. docProps/core.xml
    zip_entry_open(zip, "docProps/core.xml");
    const std::string core = get_core_xml();
    zip_entry_write(zip, core.c_str(), core.length());
    zip_entry_close(zip);

    // 5. word/_rels/document.xml.rels
    zip_entry_open(zip, "word/_rels/document.xml.rels");
    const std::string doc_rels = get_document_rels_xml();
    zip_entry_write(zip, doc_rels.c_str(), doc_rels.length());
    zip_entry_close(zip);

    // 6. word/document.xml
    zip_entry_open(zip, "word/document.xml");
    const std::string document_xml = get_empty_document_xml();
    zip_entry_write(zip, document_xml.c_str(), document_xml.length());
    zip_entry_close(zip);
}

std::string duckx::Document::get_content_types_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
    <Default Extension="xml" ContentType="application/xml"/>
    <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
    <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
    <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
</Types>)";
}

std::string duckx::Document::get_rels_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
    <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
    <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>)";
}

std::string duckx::Document::get_app_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
    <Application>DuckX</Application>
    <DocSecurity>0</DocSecurity>
    <ScaleCrop>false</ScaleCrop>
    <SharedDoc>false</SharedDoc>
    <HyperlinksChanged>false</HyperlinksChanged>
    <AppVersion>1.0</AppVersion>
</Properties>)";
}

std::string duckx::Document::get_core_xml()
{
    // 获取当前时间
    const auto now = std::time(nullptr);
    tm tm_utc{};

#if defined(_MSC_VER) // 通常 gmtime_s 在MSVC下可用
    const errno_t err = gmtime_s(&tm_utc, &now);
    if (err == 0)
    {
    }
    else
    {
        std::cerr << "Error: gmtime_s failed with code " << err << std::endl;
    }
#else
    std::cerr << "gmtime_s is not available or not checked for this compiler. Consider gmtime_r or C++20 <chrono>."
              << std::endl;
#endif
    char datetime[32];
    std::strftime(datetime, sizeof(datetime), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);

    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    <dc:creator>DuckX</dc:creator>
    <dcterms:created xsi:type="dcterms:W3CDTF">)" +
        std::string(datetime) + R"(</dcterms:created>
    <dcterms:modified xsi:type="dcterms:W3CDTF">)" +
        std::string(datetime) + R"(</dcterms:modified>
</cp:coreProperties>)";
}

std::string duckx::Document::get_document_rels_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
</Relationships>)";
}

std::string duckx::Document::get_empty_document_xml()
{
    return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
    <w:body>
    </w:body>
</w:document>)";
}

void duckx::Document::file(std::string directory)
{
    this->m_sDirectory = std::move(directory);
}

void duckx::Document::open()
{
    void* buf = nullptr;
    size_t bufsize;

    // Open file and load "xml" content to the document variable
    zip_t* zip = zip_open(this->m_sDirectory.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');

    if (!zip)
    {
        this->m_bFlag_is_open = false;
        return;
    }
    this->m_bFlag_is_open = true;

    zip_entry_open(zip, "word/document.xml");
    zip_entry_read(zip, &buf, &bufsize);

    zip_entry_close(zip);
    zip_close(zip);

    this->m_xmlDocument.load_buffer(buf, bufsize);

    free(buf);

    this->m_paragraph.set_parent(m_xmlDocument.child("w:document").child("w:body"));
}

void duckx::Document::save() const
{
    // minizip only supports appending or writing to new files
    // so we must
    // - make a new file
    // - write any new files
    // - copy the old files
    // - delete old docx
    // - rename new file to old file

    if (!this->is_open())
    {
        // if file is not existing, save() will make no sense
        return;
    }

    // Read document buffer
    xml_string_writer writer;
    this->m_xmlDocument.print(writer);

    // Open file and replace "xml" content

    const std::string original_file = this->m_sDirectory;
    const std::string temp_file = this->m_sDirectory + ".tmp";

    // Create the new file
    zip_t* new_zip = zip_open(temp_file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

    // Write out document.xml
    zip_entry_open(new_zip, "word/document.xml");

    const char* buf = writer.result.c_str();

    zip_entry_write(new_zip, buf, strlen(buf));
    zip_entry_close(new_zip);

    // Open the original zip and copy all files which are not replaced by duckX
    zip_t* orig_zip = zip_open(original_file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');

    // Loop & copy each relevant entry in the original zip
    const int orig_zip_entry_ct = zip_total_entries(orig_zip);
    for (int i = 0; i < orig_zip_entry_ct; i++)
    {
        zip_entry_openbyindex(orig_zip, i);
        const char* name = zip_entry_name(orig_zip);

        // Skip copying the original file
        if (std::string(name) != std::string("word/document.xml"))
        {
            // Read the old content
            void* entry_buf;
            size_t entry_buf_size;
            zip_entry_read(orig_zip, &entry_buf, &entry_buf_size);

            // Write into new zip
            zip_entry_open(new_zip, name);
            zip_entry_write(new_zip, entry_buf, entry_buf_size);
            zip_entry_close(new_zip);

            free(entry_buf);
        }

        zip_entry_close(orig_zip);
    }

    // Close both zips
    zip_close(orig_zip);
    zip_close(new_zip);

    // Remove original zip, rename new to correct name
    remove(original_file.c_str());
    rename(temp_file.c_str(), original_file.c_str());
}

bool duckx::Document::is_open() const
{
    return this->m_bFlag_is_open;
}

duckx::Paragraph& duckx::Document::paragraphs()
{
    pugi::xml_node body = m_xmlDocument.child("w:document").child("w:body");

    // Check if body has any paragraph nodes
    if (!body.child("w:p"))
    {
        // If no paragraphs exist, create an initial paragraph
        pugi::xml_node new_para = body.append_child("w:p");
        // Add an empty run to make it a valid paragraph
        pugi::xml_node new_run = new_para.append_child("w:r");
        const pugi::xml_node new_text = new_run.append_child("w:t");
        new_text.text().set(""); // Empty text
    }

    // Set parent and return paragraph
    this->m_paragraph.set_parent(body);
    return this->m_paragraph;
}

duckx::Table& duckx::Document::tables()
{
    this->m_table.set_parent(m_xmlDocument.child("w:document").child("w:body"));
    return this->m_table;
}