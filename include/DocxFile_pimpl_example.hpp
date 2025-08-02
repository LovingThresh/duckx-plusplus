/*!
 * @file DocxFile_pimpl_example.hpp
 * @brief Example of DocxFile refactored with Pimpl pattern for DLL safety
 * 
 * This shows how to completely eliminate C4251 warnings by hiding all
 * STL containers in the implementation. This pattern can be applied to
 * all classes that export STL members.
 */

#pragma once
#include <string>
#include "duckx_export.h"
#include "duckx_dll_config.hpp"
#include "duckx_pimpl.hpp"

struct zip_t;

namespace duckx
{
    /*!
     * @brief DLL-safe version of DocxFile using Pimpl pattern
     * 
     * All STL containers are hidden in the implementation file,
     * completely eliminating C4251 warnings.
     */
    class DUCKX_API DocxFileSafe
    {
        DUCKX_DECLARE_PIMPL(DocxFileSafe)
        
    public:
        DocxFileSafe();
        ~DocxFileSafe();

        // Non-copyable to prevent resource conflicts
        DocxFileSafe(const DocxFileSafe&) = delete;
        DocxFileSafe& operator=(const DocxFileSafe&) = delete;
        
        // Move operations
        DocxFileSafe(DocxFileSafe&&) noexcept;
        DocxFileSafe& operator=(DocxFileSafe&&) noexcept;

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
        
        /*! @brief Get the file path */
        std::string get_path() const;

    public:
        // Static methods remain unchanged
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
    };
} // namespace duckx

// Implementation would go in DocxFile_pimpl_example.cpp:
/*
#include "DocxFile_pimpl_example.hpp"
#include <map>

namespace duckx {

DUCKX_IMPL_PIMPL(DocxFileSafe) {
    std::string m_path;
    zip_t* m_zip_handle = nullptr;
    std::map<std::string, std::string> m_dirty_entries;
};

DocxFileSafe::DocxFileSafe() : pImpl(std::make_unique<Impl>()) {}
DocxFileSafe::~DocxFileSafe() = default;

DocxFileSafe::DocxFileSafe(DocxFileSafe&&) noexcept = default;
DocxFileSafe& DocxFileSafe::operator=(DocxFileSafe&&) noexcept = default;

bool DocxFileSafe::open(const std::string& path) {
    pImpl->m_path = path;
    // ... implementation ...
}

std::string DocxFileSafe::get_path() const {
    return pImpl->m_path;
}

// ... other implementations ...

} // namespace duckx
*/