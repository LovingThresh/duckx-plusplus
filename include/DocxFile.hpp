/*!
 * @file DocxFile.hpp
 * @brief Low-level DOCX file operations and ZIP archive management
 * 
 * Provides direct access to DOCX file structure, ZIP archive operations,
 * and static methods for generating standard DOCX XML templates.
 */

#pragma once
#include <map>
#include <string>
#include "duckx_export.h"

struct zip_t;

namespace duckx
{
    /*!
     * @brief Low-level DOCX file handler for ZIP archive operations
     * 
     * Manages DOCX files as ZIP archives, providing read/write access to
     * individual entries, file creation, and standard DOCX structure generation.
     * This class handles the underlying file system operations for Document.
     */
    class DUCKX_API DocxFile
    {
    public:
        DocxFile();
        ~DocxFile();

        // Non-copyable to prevent resource conflicts
        DocxFile(const DocxFile&) = delete;
        DocxFile& operator=(const DocxFile&) = delete;

        /*! @brief Open an existing DOCX file */
        bool open(const std::string& path);
        /*! @brief Create a new DOCX file with basic structure */
        bool create(const std::string& path);
        /*! @brief Save all changes to disk */
        void save();
        /*! @brief Close the file and release resources */
        void close();

        /*! @brief Check if an entry exists in the archive */
        bool has_entry(const std::string& entry_name) const;
        /*! @brief Read content from an archive entry */
        std::string read_entry(const std::string& entry_name);
        /*! @brief Write content to an archive entry */
        void write_entry(const std::string& entry_name, const std::string& content);

    public:
        // Static methods for DOCX structure generation
        /*! @brief Create basic DOCX directory structure in ZIP archive */
        static void create_basic_structure(zip_t* zip);
        /*! @brief Get standard [Content_Types].xml template */
        static std::string get_content_types_xml();
        /*! @brief Get standard app.xml template */
        static std::string get_app_xml();
        /*! @brief Get standard core.xml template */
        static std::string get_core_xml();
        /*! @brief Get standard _rels/.rels template */
        static std::string get_rels_xml();
        /*! @brief Get standard document.xml.rels template */
        static std::string get_document_rels_xml();
        /*! @brief Get empty document.xml template */
        static std::string get_empty_document_xml();
        /*! @brief Get standard styles.xml template */
        static std::string get_styles_xml();
        /*! @brief Get standard settings.xml template */
        static std::string get_settings_xml();
        /*! @brief Get standard fontTable.xml template */
        static std::string get_font_table_xml();
        /*! @brief Get standard numbering.xml template */
        static std::string get_default_numbering_xml();

        std::string m_path;                                    //!< File system path
        zip_t* m_zip_handle = nullptr;                         //!< ZIP archive handle
        std::map<std::string, std::string> m_dirty_entries;    //!< Modified entries pending write
    };
} // namespace duckx
