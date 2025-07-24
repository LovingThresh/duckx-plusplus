/*
 * @file: HeaderFooterManager.hpp
 * @brief:
 * 
 * @author: liuy
 * @date: 2025.06.09
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once

#include "duckx_export.h"

#include "constants.hpp"

#include <map>
#include <memory>

#include "pugixml.hpp"

namespace pugi
{
    class xml_document;
}

namespace duckx
{
    class Footer;
    class Header;
    class DocxFile;
    class Document;

    class DUCKX_API HeaderFooterManager
    {
    public:
        HeaderFooterManager(Document* m_owner_doc, DocxFile* file, pugi::xml_document* doc_xml,
            pugi::xml_document* rels_xml, pugi::xml_document* content_types_xml);
        ~HeaderFooterManager() = default;

        void save_all() const;
        Header& get_header(HeaderFooterType type = HeaderFooterType::DEFAULT);
        Footer& get_footer(HeaderFooterType type = HeaderFooterType::DEFAULT);

    private:
        // Generic helper for creating header or footer parts
        pugi::xml_node create_hf_part(const std::string& type_str, HeaderFooterType type_enum);
        std::string get_part_name_for_type(HeaderFooterType type, const std::string& hf_keyword) const;

        // Helper functions for XML manipulations
        std::string add_hf_relationship(const std::string& target_file, const std::string& hf_keyword_str) const;
        void add_content_type(const std::string& part_name, const std::string& content_type) const;

        void add_hf_reference_to_sect_pr(const std::string& rId, const std::string& hf_keyword, HeaderFooterType type) const;
        pugi::xml_node get_or_create_sect_pr() const;

        // Helper to convert enum to string for XML
        static std::string hf_type_to_string(HeaderFooterType type);

        DocxFile* m_file = nullptr;
        Document* m_doc = nullptr;
        pugi::xml_document* m_doc_xml = nullptr;
        pugi::xml_document* m_rels_xml = nullptr;
        pugi::xml_document* m_content_types_xml = nullptr;

        std::map<HeaderFooterType, std::unique_ptr<pugi::xml_document>> m_hf_docs;
        std::map<HeaderFooterType, std::unique_ptr<Header>> m_headers;
        std::map<HeaderFooterType, std::unique_ptr<Footer>> m_footers;

        std::map<HeaderFooterType, std::string> m_header_filenames;
        std::map<HeaderFooterType, std::string> m_footer_filenames;

        int m_header_id_counter = 1;
        int m_footer_id_counter = 1;
    };
}
