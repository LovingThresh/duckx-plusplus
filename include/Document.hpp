/*!
 * @file Document.hpp
 * @brief Main document interface for DOCX file operations
 * 
 * Provides high-level document creation, opening, and manipulation functionality
 * with both modern Result<T> API and legacy exception-based interfaces.
 * Implements the factory pattern for document creation and manages all document
 * components including body, media, headers/footers, and styles.
 * 
 * @date 2025.07
 */

#pragma once
#include <memory>
#include "duckx_export.h"
#include "Error.hpp"

#include "Body.hpp"
#include "DocxFile.hpp"
#include "HeaderFooterManager.hpp"
#include "HyperlinkManager.hpp"
#include "MediaManager.hpp"
#include "HeaderFooterBase.hpp"
#include "StyleManager.hpp"

namespace duckx
{
    class DocxFile;
    class MediaManager;
    class HeaderFooterManager;
    class HyperlinkManager;
    class Header;
    class Footer;

    /*!
     * @brief Main document class for DOCX file operations
     * 
     * Provides factory methods for creating/opening documents and access to
     * document components like body, media, and headers/footers.
     * 
     * **Example - Basic usage:**
     * @code
     * auto doc = Document::create_safe("output.docx");
     * if (doc.ok()) {
     *     doc.value().body().add_paragraph_safe("Hello World");
     *     doc.value().save_safe();
     * }
     * @endcode
     * 
     * **See Also:**
     * - @ref sample1.cpp "Basic document reading example"
     * - @ref sample2.cpp "Document creation and writing"
     * - @ref sample16_body_result_api.cpp "Modern Result<T> API usage"
     * - @ref sample_comprehensive_test.cpp "Complete workflow demonstration"
     */
    class DUCKX_API Document
    {
    public:
        // Modern Result<T> API (recommended)
        
        /*!
         * @brief Safely opens an existing DOCX document
         * @param path Path to the DOCX file to open
         * @return Result containing Document instance or error details
         */
        static Result<Document> open_safe(const std::string& path);
        
        /*!
         * @brief Safely creates a new DOCX document
         * @param path Output file path for the document
         * @return Result containing Document instance or error details
         * 
         * Creates a new document with basic structure. File is not written
         * until save_safe() is called.
         */
        static Result<Document> create_safe(const std::string& path);
        
        /*!
         * @brief Safely saves the document to disk
         * @return Result indicating success or error details
         */
        Result<void> save_safe() const;
        
        // Legacy exception-based API (for backward compatibility)
        static Document open(const std::string& path);
        static Document create(const std::string& path);
        void save() const;

        Document(Document&& other) noexcept = default;
        Document& operator=(Document&& other) noexcept = default;
        ~Document() = default;

        Body& body();
        const Body& body() const;
        MediaManager& media() const;
        HyperlinkManager& links() const;
        StyleManager& styles() const;

        std::string get_next_relationship_id();
        unsigned int get_unique_rid();

        Header& get_header(HeaderFooterType type = HeaderFooterType::DEFAULT) const;
        Footer& get_footer(HeaderFooterType type = HeaderFooterType::DEFAULT) const;
        
        // Style Set operations
        
        /*!
         * @brief Apply a style set to the document
         * @param set_name Name of the style set to apply
         * @return Result indicating success or error
         */
        Result<void> apply_style_set_safe(const std::string& set_name);
        
        /*!
         * @brief Load style definitions from an XML file
         * @param xml_file Path to XML file containing style definitions
         * @return Result indicating success or error
         */
        Result<void> load_style_definitions_safe(const std::string& xml_file);
        
        /*!
         * @brief Apply style mappings to document elements
         * @param style_mappings Map of element patterns to style names
         * @return Result indicating success or error
         */
        Result<void> apply_style_mappings_safe(const std::map<std::string, std::string>& style_mappings);
        
        /*!
         * @brief Register a new style set
         * @param style_set Style set to register
         * @return Result indicating success or error
         */
        Result<void> register_style_set_safe(const StyleSet& style_set);

    private:
        explicit Document(std::unique_ptr<DocxFile> file);
        void load();

        std::unique_ptr<DocxFile> m_file;
        pugi::xml_document m_document_xml;
        pugi::xml_document m_rels_xml;
        pugi::xml_document m_content_types_xml;

        Body m_body;
        std::unique_ptr<MediaManager> m_media_manager;
        std::unique_ptr<HeaderFooterManager> m_hf_manager;
        std::unique_ptr<HyperlinkManager> m_link_manager;
        std::unique_ptr<StyleManager> m_style_manager;
        int m_rid_counter = 1;
    };
} // namespace duckx
