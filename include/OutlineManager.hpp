/*!
 * @file OutlineManager.hpp
 * @brief Document outline and table of contents management
 * 
 * Provides functionality for generating document outlines based on heading styles,
 * creating table of contents with multi-level support, and page number updates.
 * Integrates with StyleManager for heading style recognition.
 * 
 * @date 2025.07
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "duckx_export.h"
#include "Error.hpp"
#include "constants.hpp"

namespace pugi {
    class xml_node;
}

namespace duckx {
    
    // Forward declarations
    class Document;
    class Paragraph;
    class Body;
    class StyleManager;
    
    /*!
     * @brief Entry in the document outline/TOC
     */
    struct DUCKX_API OutlineEntry {
        std::string text;                     //!< Heading text content
        int level;                            //!< Heading level (1-9)
        std::string style_name;               //!< Associated style name
        std::string bookmark_id;              //!< Internal bookmark for navigation
        absl::optional<int> page_number;      //!< Page number (if calculated)
        std::vector<OutlineEntry> children;   //!< Sub-entries for hierarchical structure
        
        OutlineEntry() : level(1) {}
        OutlineEntry(const std::string& txt, int lvl) 
            : text(txt), level(lvl) {}
    };
    
    /*!
     * @brief Table of Contents formatting options
     */
    struct DUCKX_API TocOptions {
        int max_level = 3;                    //!< Maximum heading level to include
        bool show_page_numbers = true;        //!< Whether to display page numbers
        bool use_hyperlinks = true;           //!< Create clickable links
        std::string toc_title = "Table of Contents"; //!< Title for the TOC
        std::string toc_style_prefix = "TOC"; //!< Style prefix for TOC entries
        bool right_align_page_numbers = true; //!< Right-align page numbers with leader dots
        std::string leader_char = ".";        //!< Character for page number leader
        
        TocOptions() = default;
    };
    
    /*!
     * @brief Manager for document outlines and table of contents
     * 
     * Analyzes document structure based on heading styles and generates
     * hierarchical outlines and formatted table of contents.
     * 
     * **Example usage:**
     * @code
     * auto outline_result = doc.outline_manager().generate_outline_safe();
     * if (outline_result.ok()) {
     *     auto toc_result = doc.outline_manager().create_toc_safe(TocOptions{});
     * }
     * @endcode
     */
    class DUCKX_API OutlineManager {
    public:
        /*!
         * @brief Construct outline manager for a document
         * @param doc Parent document
         * @param style_manager Style manager for heading detection
         */
        OutlineManager(Document* doc, StyleManager* style_manager);
        ~OutlineManager() = default;
        
        // Disable copy, enable move
        OutlineManager(const OutlineManager&) = delete;
        OutlineManager& operator=(const OutlineManager&) = delete;
        OutlineManager(OutlineManager&&) noexcept = default;
        OutlineManager& operator=(OutlineManager&&) noexcept = default;
        
        // ---- Outline Generation ----
        
        /*!
         * @brief Generate document outline from heading styles
         * @return Result containing outline entries or error
         */
        Result<std::vector<OutlineEntry>> generate_outline_safe();
        
        /*!
         * @brief Generate outline with custom heading detection
         * @param heading_styles List of style names to treat as headings
         * @return Result containing outline entries or error
         */
        Result<std::vector<OutlineEntry>> generate_outline_custom_safe(
            const std::vector<std::string>& heading_styles);
        
        /*!
         * @brief Get current document outline (cached)
         * @return Current outline entries
         */
        const std::vector<OutlineEntry>& get_outline() const { return m_outline; }
        
        // ---- Table of Contents ----
        
        /*!
         * @brief Create table of contents at current position
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_toc_safe(const TocOptions& options = TocOptions{});
        
        /*!
         * @brief Create Word field-based TOC (real TOC with hyperlinks)
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_field_toc_safe(const TocOptions& options = TocOptions{});
        
        /*!
         * @brief Create field-based TOC and replace placeholder text
         * @param placeholder_text Text to replace with TOC
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_field_toc_at_placeholder_safe(
            const std::string& placeholder_text,
            const TocOptions& options = TocOptions{});
        
        /*!
         * @brief Create TOC at specific paragraph position
         * @param insert_after Insert TOC after this paragraph
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_toc_at_position_safe(
            Paragraph* insert_after, 
            const TocOptions& options = TocOptions{});
        
        /*!
         * @brief Update existing table of contents
         * @return Result indicating success or error
         */
        Result<void> update_toc_safe();
        
        /*!
         * @brief Remove table of contents from document
         * @return Result indicating success or error
         */
        Result<void> remove_toc_safe();
        
        // ---- Page Number Management ----
        
        /*!
         * @brief Calculate page numbers for outline entries
         * @return Result indicating success or error
         * 
         * Note: This is a simplified estimation as exact page calculation
         * requires full document rendering
         */
        Result<void> calculate_page_numbers_safe();
        
        /*!
         * @brief Update page numbers in existing TOC
         * @return Result indicating success or error
         */
        Result<void> update_page_numbers_safe();
        
        // ---- Heading Style Management ----
        
        /*!
         * @brief Register a style as a heading with specific level
         * @param style_name Style name to register
         * @param level Heading level (1-9)
         * @return Result indicating success or error
         */
        Result<void> register_heading_style_safe(
            const std::string& style_name, 
            int level);
        
        /*!
         * @brief Get heading level for a style
         * @param style_name Style name to check
         * @return Heading level (1-9) or nullopt if not a heading
         */
        absl::optional<int> get_heading_level(const std::string& style_name) const;
        
        /*!
         * @brief Check if a style is registered as a heading
         * @param style_name Style name to check
         * @return true if style is a heading style
         */
        bool is_heading_style(const std::string& style_name) const;
        
        // ---- Outline Navigation ----
        
        /*!
         * @brief Find outline entry by text
         * @param text Text to search for
         * @return Pointer to entry or nullptr if not found
         */
        const OutlineEntry* find_entry_by_text(const std::string& text) const;
        
        /*!
         * @brief Get outline as flat list (depth-first traversal)
         * @return Flattened outline entries
         */
        std::vector<const OutlineEntry*> get_flat_outline() const;
        
        // ---- Export Functions ----
        
        /*!
         * @brief Export outline to plain text format
         * @param indent_size Spaces per indent level
         * @return Result containing text representation or error
         */
        Result<std::string> export_outline_as_text_safe(int indent_size = 2) const;
        
        /*!
         * @brief Export outline to HTML format
         * @return Result containing HTML representation or error
         */
        Result<std::string> export_outline_as_html_safe() const;
        
    private:
        Document* m_document;
        StyleManager* m_style_manager;
        std::vector<OutlineEntry> m_outline;
        std::map<std::string, int> m_heading_styles; // style_name -> level
        bool m_toc_exists = false;
        pugi::xml_node* m_toc_node = nullptr;
        
        // Helper methods
        Result<void> scan_document_for_headings_safe();
        Result<void> build_hierarchy_safe(std::vector<OutlineEntry>& entries);
        Result<void> create_toc_paragraph_safe(
            const OutlineEntry& entry,
            const TocOptions& options,
            Body& body);
        Result<void> create_toc_entry_recursive_safe(
            const OutlineEntry& entry,
            const TocOptions& options,
            Body& body);
        Result<void> apply_toc_styles_safe();
        void flatten_outline_recursive(
            const std::vector<OutlineEntry>& entries,
            std::vector<const OutlineEntry*>& flat_list) const;
        Result<std::string> generate_bookmark_id_safe() const;
        Result<void> estimate_page_number_safe(OutlineEntry& entry, int current_page) const;
        
        // Field TOC helpers
        Result<void> add_bookmarks_to_headings_safe();
        Result<void> create_toc_styles_safe();
        Result<pugi::xml_node> create_sdt_toc_node_safe(
            pugi::xml_node parent_node,
            const TocOptions& options);
    };
    
} // namespace duckx