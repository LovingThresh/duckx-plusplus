/*!
 * @file MediaManager.hpp
 * @brief Manager class for media elements in DOCX documents
 * 
 * Handles embedding of media files (images, textboxes) into DOCX documents
 * including file management, relationship creation, and XML generation.
 */

#pragma once
#include <string>
#include "duckx_export.h"

// Forward declarations to keep headers clean
namespace pugi
{
    class xml_document;
    class xml_node;
} // namespace pugi

namespace duckx
{
    class Document;
    class DocxFile;
    class Image;
    class TextBox;
    class Paragraph;
    class Run;

    /*!
     * @brief Manager for media elements in documents
     * 
     * Manages the embedding of media files such as images and textboxes
     * into DOCX documents. Handles file storage, relationship creation,
     * and XML structure generation for proper media integration.
     */
    class DUCKX_API MediaManager
    {
    public:
        MediaManager(Document* owner_doc, DocxFile* file, pugi::xml_document* rels_xml, pugi::xml_document* doc_xml,
                     pugi::xml_document* content_types_xml);

        /*! @brief Add an image to a paragraph and return the created run */
        Run add_image(const Paragraph& p, const Image& image);
        /*! @brief Add a textbox to a paragraph and return the created run */
        Run add_textbox(const Paragraph& p, const TextBox& textbox);

    private:
        /*! @brief Add media file to the DOCX archive */
        std::string add_media_to_zip(const std::string& file_path);
        /*! @brief Create relationship entry for image */
        std::string add_image_relationship(const std::string& media_target) const;
        /*! @brief Generate unique document property ID */
        unsigned int get_unique_docpr_id();

        DocxFile* m_file = nullptr;                         //!< DOCX file handler
        pugi::xml_document* m_rels_xml = nullptr;           //!< Relationships XML
        pugi::xml_document* m_doc_xml = nullptr;            //!< Document XML
        pugi::xml_document* m_content_types_xml = nullptr;  //!< Content types XML

        Document* m_doc = nullptr;           //!< Reference to parent document
        int m_media_id_counter = 1;         //!< Counter for media file IDs
        unsigned int m_docpr_id_counter = 1; //!< Counter for document property IDs
    };
} // namespace duckx
