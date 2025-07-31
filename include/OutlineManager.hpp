/*!
 * @file OutlineManager_unified.hpp
 * @brief Unified document outline and table of contents management with cross-platform safety
 * 
 * Provides functionality for generating document outlines based on heading styles,
 * creating table of contents with multi-level support, and page number updates.
 * Integrates with StyleManager for heading style recognition.
 * 
 * This unified version combines all features with Windows MSVC/Clang compatibility.
 * 
 * @date 2025.08
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <exception>

#include "duckx_export.h"
#include "Error.hpp"
#include "constants.hpp"

// Forward declarations for pugixml
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
     * @brief Cross-platform safe outline node for hierarchical structure
     */
    class DUCKX_API OutlineNode : public std::enable_shared_from_this<OutlineNode> {
    public:
        using Ptr = std::shared_ptr<OutlineNode>;
        using WeakPtr = std::weak_ptr<OutlineNode>;
        
    private:
        std::string m_name;
        std::vector<Ptr> m_children;
        WeakPtr m_parent;
        mutable std::mutex m_mutex;
        
    public:
        OutlineNode() = default;
        explicit OutlineNode(const std::string& name) : m_name(name) {}
        
        ~OutlineNode() {
            // 使用try-catch防止析构函数中的异常
            try {
                std::unique_lock<std::mutex> lock(m_mutex, std::try_to_lock);
                if (lock.owns_lock()) {
                    // 先断开父子关系避免循环引用
                    for (auto& child : m_children) {
                        if (child) {
                            try {
                                child->m_parent.reset();
                            } catch (...) {
                                // 忽略析构时的异常
                            }
                        }
                    }
                    m_children.clear();
                    m_parent.reset();
                }
            } catch (...) {
                // 析构函数不应该抛出异常
            }
        }
        
        std::string getName() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_name;
        }
        
        void setName(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_name = name;
        }
        
        void addChild(Ptr child) {
            if (!child) return;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            m_children.push_back(child);
            
            // Safely set parent
            try {
                child->m_parent = shared_from_this();
            } catch (const std::bad_weak_ptr&) {
                // This node is not managed by shared_ptr, skip parent assignment
            }
        }
        
        std::vector<Ptr> getChildren() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_children;
        }
        
        Ptr getParent() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_parent.lock();
        }
        
        void clearChildren() {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& child : m_children) {
                if (child) {
                    try {
                        child->m_parent.reset();
                    } catch (...) {
                        // 忽略清理过程中的异常
                    }
                }
            }
            m_children.clear();
        }
        
        size_t getChildCount() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_children.size();
        }
    };
    
    /*!
     * @brief Unified manager for document outlines and table of contents
     * 
     * Analyzes document structure based on heading styles and generates
     * hierarchical outlines and formatted table of contents.
     * 
     * Features cross-platform compatibility with Windows MSVC/Clang safety measures.
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
        
        /*!
         * @brief 安全析构函数，防止segmentation fault
         */
        ~OutlineManager() {
            try {
                clear();
            } catch (...) {
                // 析构函数不应该抛出异常
            }
        }
        
        // Disable copy and move (due to std::mutex member)
        OutlineManager(const OutlineManager&) = delete;
        OutlineManager& operator=(const OutlineManager&) = delete;
        OutlineManager(OutlineManager&&) = delete;
        OutlineManager& operator=(OutlineManager&&) = delete;
        
        // ---- Outline Generation ----
        
        /*!
         * @brief Generate document outline from heading styles (cross-platform safe)
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
         * @brief Create table of contents at current position (cross-platform safe)
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
            const Paragraph& insert_after,
            const TocOptions& options = TocOptions{});
        
        /*!
         * @brief Update existing table of contents
         * @return Result indicating success or error
         */
        Result<void> update_toc_safe();
        
        /*!
         * @brief Remove existing table of contents
         * @return Result indicating success or error
         */
        Result<void> remove_toc_safe();
        
        // ---- Page Number Management ----
        
        /*!
         * @brief Calculate page numbers for outline entries
         * @return Result indicating success or error
         */
        Result<void> calculate_page_numbers_safe();
        
        /*!
         * @brief Update page numbers in existing TOC
         * @return Result indicating success or error
         */
        Result<void> update_page_numbers_safe();
        
        // ---- Heading Style Management ----
        
        /*!
         * @brief Register a style as a heading style
         * @param style_name Name of the style
         * @param level Heading level (1-9)
         * @return Result indicating success or error
         */
        Result<void> register_heading_style_safe(const std::string& style_name, int level);
        
        /*!
         * @brief Get heading level for a style
         * @param style_name Style name to check
         * @return Heading level if registered, nullopt otherwise
         */
        absl::optional<int> get_heading_level(const std::string& style_name) const;
        
        /*!
         * @brief Check if a style is registered as a heading
         * @param style_name Style name to check
         * @return True if style is a heading style
         */
        bool is_heading_style(const std::string& style_name) const;
        
        // ---- Outline Navigation ----
        
        /*!
         * @brief Find outline entry by bookmark ID
         * @param bookmark_id Bookmark identifier
         * @return Pointer to entry if found, nullptr otherwise
         */
        const OutlineEntry* find_entry_by_bookmark(const std::string& bookmark_id) const;
        
        /*!
         * @brief Get flattened list of all outline entries
         * @return Vector of pointers to all entries in document order
         */
        std::vector<const OutlineEntry*> get_flat_outline() const;
        
        // ---- Export Functions ----
        
        /*!
         * @brief Export outline as formatted text
         * @param indent_size Number of spaces per indentation level
         * @return Result containing text representation or error
         */
        Result<std::string> export_outline_as_text_safe(int indent_size = 2) const;
        
        /*!
         * @brief Export outline as HTML list
         * @return Result containing HTML representation or error
         */
        Result<std::string> export_outline_as_html_safe() const;
        
        // ---- Simple API for Testing/Basic Use ----
        
        /*!
         * @brief Create a simple TOC with mock data (for testing/demos)
         * @return True if successful
         */
        bool createTOC();
        
        /*!
         * @brief Get number of outline nodes
         * @return Count of outline nodes
         */
        size_t getOutlineNodeCount() const;
        
        /*!
         * @brief Get body node name by index (safe cross-platform access)
         * @param index Index of the node
         * @return Node name or empty string if invalid
         */
        std::string getBodyNodeName(size_t index) const;
        
        /*!
         * @brief Get outline node by index
         * @param index Index of the node
         * @return Shared pointer to node or nullptr if invalid
         */
        OutlineNode::Ptr getOutlineNode(size_t index) const;
        
        /*!
         * @brief Clear all outline data
         */
        void clear();
        
    private:
        // Core data members
        Document* m_document;
        StyleManager* m_style_manager;
        std::vector<OutlineEntry> m_outline;
        std::map<std::string, int> m_heading_styles;
        bool m_toc_exists = false;
        
        // Cross-platform safe storage
        std::vector<OutlineNode::Ptr> m_outline_nodes;
        mutable std::mutex m_mutex;
        
        // ---- Private Helper Methods ----
        
        /*!
         * @brief Scan document for headings (cross-platform safe implementation)
         * @return Result indicating success or error
         */
        Result<void> scan_document_for_headings_safe();
        
        /*!
         * @brief Build hierarchical structure from flat list
         * @param entries Flat list of entries to organize
         * @return Result indicating success or error
         */
        Result<void> build_hierarchy_safe(std::vector<OutlineEntry>& entries);
        
        /*!
         * @brief Create TOC paragraph with formatting
         * @param entry Outline entry to create paragraph for
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_toc_paragraph_safe(
            const OutlineEntry& entry, 
            const TocOptions& options);
        
        /*!
         * @brief Create TOC entry recursively for children
         * @param parent_entry Parent entry
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_toc_entry_recursive_safe(
            const OutlineEntry& parent_entry,
            const TocOptions& options);
        
        /*!
         * @brief Create individual TOC entry
         * @param entry Entry to create
         * @param options Formatting options
         * @return Result indicating success or error
         */
        Result<void> create_toc_entry_safe(
            const OutlineEntry& entry,
            const TocOptions& options);
        
        /*!
         * @brief Apply TOC-specific styles
         * @return Result indicating success or error
         */
        Result<void> apply_toc_styles_safe();
        
        /*!
         * @brief Flatten outline recursively for navigation
         * @param entries Source entries
         * @param flat_list Output flattened list
         */
        void flatten_outline_recursive(
            const std::vector<OutlineEntry>& entries,
            std::vector<const OutlineEntry*>& flat_list) const;
        
        /*!
         * @brief Generate unique bookmark ID
         * @return Result containing bookmark ID or error
         */
        Result<std::string> generate_bookmark_id_safe() const;
        
        /*!
         * @brief Estimate page number for entry
         * @param entry Entry to estimate page for
         * @param current_page Current page number
         * @return Result indicating success or error
         */
        Result<void> estimate_page_number_safe(OutlineEntry& entry, int current_page) const;
        
        /*!
         * @brief Create Word field-based TOC structure
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_field_toc_internal_safe(const TocOptions& options);
        
        /*!
         * @brief Add bookmarks to heading paragraphs
         * @return Result indicating success or error
         */
        Result<void> add_bookmarks_to_headings_safe();
        
        /*!
         * @brief Create TOC-specific styles if needed
         * @return Result indicating success or error
         */
        Result<void> create_toc_styles_safe();
        
        /*!
         * @brief Create structured document tag for TOC
         * @param options TOC formatting options
         * @return Result containing XML node or error
         */
        Result<pugi::xml_node> create_sdt_toc_node_safe(const TocOptions& options) const;
        
        /*!
         * @brief Create field-based TOC at placeholder location
         * @param placeholder_text Text to replace
         * @param options TOC formatting options
         * @return Result indicating success or error
         */
        Result<void> create_field_toc_at_placeholder_internal_safe(
            const std::string& placeholder_text,
            const TocOptions& options);
    };
    
} // namespace duckx