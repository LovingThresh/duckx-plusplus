/*!
 * @file HeaderFooterManager.hpp
 * @brief Manager class for document headers and footers
 * 
 * Handles creation, management, and persistence of headers and footers
 * in DOCX documents with support for different header/footer types.
 * Provides comprehensive header/footer functionality with XML serialization.
 * 
 * @date 2025.07
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

    /*!
     * @brief Manager for document headers and footers
     * 
     * Handles the creation, management, and XML serialization of headers
     * and footers. Supports different types like default, first page, and
     * even/odd page headers/footers.
     */
    class DUCKX_API HeaderFooterManager
    {
    public:
        HeaderFooterManager(Document* m_owner_doc, DocxFile* file, pugi::xml_document* doc_xml,
            pugi::xml_document* rels_xml, pugi::xml_document* content_types_xml);
        ~HeaderFooterManager() = default;

        /*! @brief Save all headers and footers to the document */
        void save_all() const;
        /*! @brief Get or create a header of the specified type */
        Header& get_header(HeaderFooterType type = HeaderFooterType::DEFAULT);
        /*! @brief Get or create a footer of the specified type */
        Footer& get_footer(HeaderFooterType type = HeaderFooterType::DEFAULT);

    private:
        // Generic helper for creating header or footer parts
        /*! @brief Create XML structure for header or footer part */
        pugi::xml_node create_hf_part(const std::string& type_str, HeaderFooterType type_enum);
        /*! @brief Generate part name based on type and keyword */
        std::string get_part_name_for_type(HeaderFooterType type, const std::string& hf_keyword) const;

        // Helper functions for XML manipulations
        /*! @brief Add relationship entry for header/footer */
        std::string add_hf_relationship(const std::string& target_file, const std::string& hf_keyword_str) const;
        /*! @brief Add content type entry for header/footer */
        void add_content_type(const std::string& part_name, const std::string& content_type) const;

        /*! @brief Add header/footer reference to section properties */
        void add_hf_reference_to_sect_pr(const std::string& rId, const std::string& hf_keyword, HeaderFooterType type) const;
        /*! @brief Get or create section properties node */
        pugi::xml_node get_or_create_sect_pr() const;

        // Helper to convert enum to string for XML
        /*! @brief Convert HeaderFooterType enum to string */
        static std::string hf_type_to_string(HeaderFooterType type);

        DocxFile* m_file = nullptr;                    //!< Reference to DOCX file handler
        Document* m_doc = nullptr;                     //!< Reference to parent document
        pugi::xml_document* m_doc_xml = nullptr;       //!< Document XML
        pugi::xml_document* m_rels_xml = nullptr;      //!< Relationships XML
        pugi::xml_document* m_content_types_xml = nullptr; //!< Content types XML

        std::map<HeaderFooterType, std::unique_ptr<pugi::xml_document>> m_hf_docs; //!< XML documents for headers/footers
        std::map<HeaderFooterType, std::unique_ptr<Header>> m_headers;             //!< Header instances by type
        std::map<HeaderFooterType, std::unique_ptr<Footer>> m_footers;             //!< Footer instances by type

        std::map<HeaderFooterType, std::string> m_header_filenames; //!< Header file names by type
        std::map<HeaderFooterType, std::string> m_footer_filenames; //!< Footer file names by type

        int m_header_id_counter = 1; //!< Counter for generating header IDs
        int m_footer_id_counter = 1; //!< Counter for generating footer IDs
    };
}
