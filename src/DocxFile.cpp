/*
 * @file: DocFile.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "DocxFile.hpp"

#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#endif

#include <ctime>

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
        if (m_dirty_entries.count(entry_name))
        {
            return true;
        }

        zip_t* zip = zip_open(m_path.c_str(), 0, 'r');
        if (!zip)
            return false;

        const bool found = (zip_entry_open(zip, entry_name.c_str()) == 0);

        if (found)
            zip_entry_close(zip);
        zip_close(zip);

        return found;
    }

    std::string DocxFile::read_entry(const std::string& entry_name)
    {
        // 优先从已修改的缓存中读取
        if (m_dirty_entries.count(entry_name))
        {
            return m_dirty_entries[entry_name];
        }

        zip_t* zip = zip_open(m_path.c_str(), 0, 'r');
        if (!zip)
        {
            // 如果文件不存在但我们想读取一个空文档，就返回空文档XML
            if (entry_name == "word/document.xml")
                return get_empty_document_xml();
            throw std::runtime_error("Failed to open zip file: " + m_path);
        }

        if (zip_entry_open(zip, entry_name.c_str()) != 0)
        {
            zip_close(zip);
            if (entry_name == "word/document.xml")
                return get_empty_document_xml();
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
        for (const auto& pair: m_dirty_entries)
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
        // 步骤 1. [Content_Types].xml
        zip_entry_open(zip, "[Content_Types].xml");
        const std::string content_types = get_content_types_xml();
        zip_entry_write(zip, content_types.c_str(), content_types.length());
        zip_entry_close(zip);

        // 步骤 2. _rels/.rels
        zip_entry_open(zip, "_rels/.rels");
        const std::string rels = get_rels_xml();
        zip_entry_write(zip, rels.c_str(), rels.length());
        zip_entry_close(zip);

        // 步骤 3. docProps/app.xml
        zip_entry_open(zip, "docProps/app.xml");
        const std::string app = get_app_xml();
        zip_entry_write(zip, app.c_str(), app.length());
        zip_entry_close(zip);

        // 步骤 4. docProps/core.xml
        zip_entry_open(zip, "docProps/core.xml");
        const std::string core = get_core_xml();
        zip_entry_write(zip, core.c_str(), core.length());
        zip_entry_close(zip);

        // 步骤 5: word/document.xml (先创建主文档)
        zip_entry_open(zip, "word/document.xml");
        const std::string document_xml = get_empty_document_xml();
        zip_entry_write(zip, document_xml.c_str(), document_xml.length());
        zip_entry_close(zip);

        // 步骤 6: word/styles.xml
        zip_entry_open(zip, "word/styles.xml");
        const std::string styles = get_styles_xml(); // 调用新函数
        zip_entry_write(zip, styles.c_str(), styles.length());
        zip_entry_close(zip);

        // 步骤 7: word/settings.xml
        zip_entry_open(zip, "word/settings.xml");
        const std::string settings = get_settings_xml(); // 调用新函数
        zip_entry_write(zip, settings.c_str(), settings.length());
        zip_entry_close(zip);

        // 步骤 8: word/fontTable.xml
        zip_entry_open(zip, "word/fontTable.xml");
        const std::string font_table = get_font_table_xml(); // 调用新函数
        zip_entry_write(zip, font_table.c_str(), font_table.length());
        zip_entry_close(zip);

        // 步骤 9: word/numbering.xml
        zip_entry_open(zip, "word/numbering.xml");
        const std::string numbering = get_default_numbering_xml();
        zip_entry_write(zip, numbering.c_str(), numbering.length());
        zip_entry_close(zip);

        // 步骤 10 : word/_rels/document.xml.rels
        zip_entry_open(zip, "word/_rels/document.xml.rels");
        const std::string doc_rels = get_document_rels_xml(); // 确保这个函数现在是完整的
        zip_entry_write(zip, doc_rels.c_str(), doc_rels.length());
        zip_entry_close(zip);
    }

    std::string DocxFile::get_content_types_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                // 如果要支持图片，必须有下面这几行
                "<Default Extension=\"png\" ContentType=\"image/png\"/>"
                "<Default Extension=\"jpg\" ContentType=\"image/jpeg\"/>"
                "<Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/>"
                // --- 以下是针对 Word 核心文件的 Override ---
                "<Override PartName=\"/word/document.xml\" "
                "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
                "<Override PartName=\"/word/styles.xml\" "
                "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
                "<Override PartName=\"/word/settings.xml\" "
                "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml\"/>"
                "<Override PartName=\"/word/fontTable.xml\" "
                "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml\"/>"
                "<Override PartName=\"/docProps/core.xml\" "
                "ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
                "<Override PartName=\"/docProps/app.xml\" "
                "ContentType=\"application/vnd.openxmlformats-officedocument.extended-properties+xml\"/>"
                "<Override PartName=\"/word/numbering.xml\" "
                "ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml\"/>"
                "</Types>";
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
        tm tm_utc{};

#if defined(_WIN32)
        // 在Windows平台，使用Win32 API GetSystemTime() 以获得最佳兼容性。
        // 此API在所有现代和旧版Windows中都可用，且无C运行时库依赖问题。
        SYSTEMTIME st;
        GetSystemTime(&st);

        tm_utc.tm_year = st.wYear - 1900;
        tm_utc.tm_mon = st.wMonth - 1;
        tm_utc.tm_mday = st.wDay;
        tm_utc.tm_hour = st.wHour;
        tm_utc.tm_min = st.wMinute;
        tm_utc.tm_sec = st.wSecond;
        tm_utc.tm_wday = st.wDayOfWeek;
        tm_utc.tm_isdst = 0; // UTC没有夏令时

        // 计算一年中的第几天 (tm_yday)
        constexpr int daysBeforeMonth[13] = {
                0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
        };
        tm_utc.tm_yday = daysBeforeMonth[tm_utc.tm_mon] + tm_utc.tm_mday - 1;
        auto isLeapYear = [](int year)
        {
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        };
        if (tm_utc.tm_mon > 1 && isLeapYear(st.wYear))
        {
            tm_utc.tm_yday++;
        }

#else
        // 在类UNIX系统（Linux, macOS等），使用线程安全的 gmtime_r
        const auto now = std::time(nullptr);
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
        gmtime_r(&now, &tm_utc);
#else
        // 为其他平台提供一个非线程安全的备选方案，保证可编译性。
        tm* temp_tm = std::gmtime(&now);
        if (temp_tm != nullptr)
        {
            tm_utc = *temp_tm;
        }
#endif // __unix__ || __APPLE__ || __linux__
#endif // _WIN32
        char datetime[32];
        std::strftime(datetime, sizeof(datetime), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);

        std::ostringstream oss;
        oss << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)"
                <<
                R"(<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">)"
                << "<dc:creator>DuckX</dc:creator>"
                << "<dcterms:created xsi:type=\"dcterms:W3CDTF\">" << datetime << "</dcterms:created>"
                << "<dcterms:modified xsi:type=\"dcterms:W3CDTF\">" << datetime << "</dcterms:modified>"
                << "</cp:coreProperties>";

        return oss.str();
    }

    std::string DocxFile::get_document_rels_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                "<Relationship Id=\"rId3\" "
                "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" "
                "Target=\"styles.xml\"/>"
                "<Relationship Id=\"rId2\" "
                "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings\" "
                "Target=\"settings.xml\"/>"
                "<Relationship Id=\"rId1\" "
                "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable\" "
                "Target=\"fontTable.xml\"/>"
                "<Relationship Id=\"rId4\" "
                "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering\" "
                "Target=\"numbering.xml\"/>"
                "</Relationships>";
    }

    std::string DocxFile::get_empty_document_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<w:document "
                "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" "
                "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" "
                "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\">"
                "  <w:body>"
                "  </w:body>"
                "</w:document>";
    }

    std::string DocxFile::get_styles_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
                "  <w:docDefaults>"
                "    <w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz "
                "w:val=\"24\"/></w:rPr></w:rPrDefault>"
                "    <w:pPrDefault><w:pPr><w:spacing w:after=\"200\" w:line=\"276\" "
                "w:lineRule=\"auto\"/></w:pPr></w:pPrDefault>"
                "  </w:docDefaults>"
                "  <w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\">"
                "    <w:name w:val=\"Normal\"/>"
                "  </w:style>"
                "</w:styles>";
    }

    std::string DocxFile::get_settings_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<w:settings xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
                "  <w:zoom w:percent=\"100\"/>"
                "</w:settings>";
    }

    std::string DocxFile::get_font_table_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<w:fonts xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
                "  <w:font w:name=\"Times New Roman\">"
                "    <w:panose1 w:val=\"02020603050405020304\"/>"
                "  </w:font>"
                "</w:fonts>";
    }

    std::string DocxFile::get_default_numbering_xml()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                "<w:numbering xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
                "  <w:abstractNum w:abstractNumId=\"0\">"
                "    <w:lvl w:ilvl=\"0\">"
                "      <w:start w:val=\"1\"/>"
                "      <w:numFmt w:val=\"bullet\"/>"
                "      <w:lvlText w:val=\"\xE2\x80\xA2\"/>" // UTF-8编码的实心圆点 (•)
                "      <w:lvlJc w:val=\"left\"/>"
                "      <w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr>"
                "      <w:rPr><w:rFonts w:hint=\"default\"/></w:rPr>" // 简化rFonts
                "    </w:lvl>"
                "  </w:abstractNum>"
                "  <w:abstractNum w:abstractNumId=\"1\">"
                "    <w:lvl w:ilvl=\"0\">"
                "      <w:start w:val=\"1\"/>"
                "      <w:numFmt w:val=\"decimal\"/>"
                "      <w:lvlText w:val=\"%1.\"/>"
                "      <w:lvlJc w:val=\"left\"/>"
                "      <w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr>"
                "    </w:lvl>"
                "  </w:abstractNum>"
                "  <w:num w:numId=\"1\">"
                "    <w:abstractNumId w:val=\"0\"/>"
                "  </w:num>"
                "  <w:num w:numId=\"2\">"
                "    <w:abstractNumId w:val=\"1\"/>"
                "  </w:num>"
                "</w:numbering>";
    }
} // namespace duckx
