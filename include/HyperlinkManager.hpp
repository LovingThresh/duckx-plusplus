/*!
 * @file HyperlinkManager.hpp
 * @brief Manager class for document hyperlinks and external relationships
 * 
 * Handles the creation and management of hyperlinks in DOCX documents,
 * including URL relationships and XML structure generation.
 */

#pragma once

#include <string>
#include "duckx_export.h"
#include "pugixml.hpp"

namespace duckx
{
    class Document;

    /*!
     * @brief Manager for document hyperlinks
     * 
     * Manages hyperlink relationships in DOCX documents by creating
     * external relationships in the document's relationships XML and
     * generating unique relationship IDs for hyperlinks.
     */
    class DUCKX_API HyperlinkManager
    {
    public:
        HyperlinkManager(Document* doc, pugi::xml_document* rels_xml);

        /*! @brief Add a hyperlink relationship and return its ID */
        std::string add_relationship(const std::string& url) const;

    private:
        Document* m_doc = nullptr;               //!< Reference to parent document
        pugi::xml_document* m_rels_xml = nullptr; //!< Document relationships XML
    };
}