/*
 * @file: DocFile.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>


#include <stdexcept>
#include "DocxFile.hpp"
#include "zip.h"

namespace duckx
{
    DocxFile::DocxFile() = default;

    DocxFile::~DocxFile() = default;

    bool DocxFile::open(const std::string& path)
    {
        m_path = path;
        zip_t* zip = zip_open(m_path.c_str(), 0, 'r');
        if (!zip)
        {
            return false;
        }
        zip_close(zip); // 仅检查文件是否存在且可读，不保持打开
        return true;
    }

    bool DocxFile::create(const std::string& path)
    {
        m_path = path;
        zip_t* zip = zip_open(path.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
        if (!zip)
        {
            return false;
        }
        create_basic_structure(zip);
        zip_close(zip);
        return true;
    }

    void DocxFile::close()
    {
        // 因为我们不是一直保持文件打开，这个方法可以为空，或者用于清理资源
        m_path.clear();
        m_dirty_entries.clear();
    }

    bool DocxFile::has_entry(const std::string& entry_name) const
    {
        if (m_dirty_entries.count(entry_name)) {
            return true;
        }

        zip_t* zip = zip_open(m_path.c_str(), 0, 'r');
        if (!zip)
            return false;

        const bool found = (zip_entry_open(zip, entry_name.c_str()) == 0);

        if(found) zip_entry_close(zip);
        zip_close(zip);

        return found;
    }

    std::string DocxFile::read_entry(const std::string& entry_name)
    {
        // 优先从已修改的缓存中读取
        if (m_dirty_entries.count(entry_name)) {
            return m_dirty_entries[entry_name];
        }

        zip_t* zip = zip_open(m_path.c_str(), 0, 'r');
        if (!zip) {
            // 如果文件不存在但我们想读取一个空文档，就返回空文档XML
            if(entry_name == "word/document.xml") return get_empty_document_xml();
            throw std::runtime_error("Failed to open zip file: " + m_path);
        }

        if (zip_entry_open(zip, entry_name.c_str()) != 0) {
            zip_close(zip);
            if(entry_name == "word/document.xml") return get_empty_document_xml();
            throw std::runtime_error("Failed to open zip entry: " + entry_name);
        }

        void* buf = nullptr;
        size_t buf_size;
        zip_entry_read(zip, &buf, &buf_size);
        std::string content(static_cast<char*>(buf), buf_size);

        free(buf);
        zip_entry_close(zip);
        zip_close(zip);

        return content;
    }

    void DocxFile::write_entry(const std::string& entry_name, const std::string& content)
    {
        m_dirty_entries[entry_name] = content;
    }

    void DocxFile::save()
    {
        if (m_path.empty())
        {
            throw std::runtime_error("File path is not set. Cannot save.");
        }

        const std::string temp_file = m_path + ".tmp";

        // 创建临时zip文件
        zip_t* new_zip = zip_open(temp_file.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
        if (!new_zip)
        {
            throw std::runtime_error("Failed to create temporary zip file.");
        }

        // 写入所有被修改或新添加的文件
        for (const auto& pair : m_dirty_entries)
        {
            zip_entry_open(new_zip, pair.first.c_str());
            zip_entry_write(new_zip, pair.second.c_str(), pair.second.length());
            zip_entry_close(new_zip);
        }

        // 打开原始zip文件，拷贝所有未被修改的文件
        zip_t* orig_zip = zip_open(m_path.c_str(), 0, 'r');
        if (orig_zip)
        {
            const int entry_count = zip_total_entries(orig_zip);
            for (int i = 0; i < entry_count; ++i)
            {
                zip_entry_openbyindex(orig_zip, i);
                const char* name = zip_entry_name(orig_zip);

                // 如果这个文件没被修改过，就从旧文件拷贝到新文件
                if (m_dirty_entries.find(name) == m_dirty_entries.end())
                {
                    void* buf = nullptr;
                    size_t buf_size;
                    zip_entry_read(orig_zip, &buf, &buf_size);

                    zip_entry_open(new_zip, name);
                    zip_entry_write(new_zip, buf, buf_size);
                    zip_entry_close(new_zip);
                    free(buf);
                }
                zip_entry_close(orig_zip);
            }
            zip_close(orig_zip);
        }

        zip_close(new_zip);

        // 替换原始文件
        remove(m_path.c_str());
        rename(temp_file.c_str(), m_path.c_str());
    }

    void DocxFile::create_basic_structure(zip_t* zip)
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

    std::string DocxFile::get_content_types_xml()
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

    std::string DocxFile::get_rels_xml()
    {
        return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
    <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
    <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>)";
    }

    std::string DocxFile::get_app_xml()
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

    std::string DocxFile::get_core_xml()
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

    std::string DocxFile::get_document_rels_xml()
    {
        return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
</Relationships>)";
    }

    std::string DocxFile::get_empty_document_xml()
    {
        return R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
    <w:body>
    </w:body>
</w:document>)";
    }
}