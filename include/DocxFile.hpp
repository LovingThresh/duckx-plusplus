/*
 * @file: DocxFile.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include <map>
#include <string>
#include "duckx_export.h"

struct zip_t;

namespace duckx
{
    class DUCKX_API DocxFile
    {
    public:
        DocxFile();
        ~DocxFile();

        // 禁止拷贝和赋值
        DocxFile(const DocxFile&) = delete;
        DocxFile& operator=(const DocxFile&) = delete;

        bool open(const std::string& path);
        bool create(const std::string& path);
        void save();
        void close();

        bool has_entry(const std::string& entry_name) const;
        std::string read_entry(const std::string& entry_name);
        void write_entry(const std::string& entry_name, const std::string& content);

    public:
        static void create_basic_structure(zip_t* zip);
        static std::string get_content_types_xml();
        static std::string get_app_xml();
        static std::string get_core_xml();
        static std::string get_rels_xml();
        static std::string get_document_rels_xml();
        static std::string get_empty_document_xml();
        static std::string get_styles_xml();
        static std::string get_settings_xml();
        static std::string get_font_table_xml();
        static std::string get_default_numbering_xml();

        std::string m_path;
        zip_t* m_zip_handle = nullptr;
        std::map<std::string, std::string> m_dirty_entries;
    };
} // namespace duckx
