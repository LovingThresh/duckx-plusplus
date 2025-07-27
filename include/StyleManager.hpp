/*!
 * @file StyleManager.hpp
 * @brief Style management system for DOCX documents
 * 
 * Provides a comprehensive style management system with support for paragraph styles,
 * character styles, table styles, and built-in style libraries. Integrates with the
 * modern Result<T> error handling system for robust style operations.
 */
#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <unordered_set>

#include "Error.hpp"
#include "constants.hpp"
#include "duckx_export.h"
#include "pugixml.hpp"

namespace duckx
{
    // Forward declarations
    class DocxElement;
    class Paragraph;
    class Run;
    class Table;
    
    // ============================================================================
    // Style Type Definitions
    // ============================================================================
    
    /*!
     * @brief Types of styles supported by the style system
     */
    enum class DUCKX_API StyleType
    {
        PARAGRAPH,   //!< Paragraph-level styles (alignment, spacing, indentation)
        CHARACTER,   //!< Character-level styles (font, size, formatting)
        TABLE,       //!< Table-level styles (borders, shading, layout)
        NUMBERING,   //!< Numbering and list styles
        MIXED        //!< Mixed styles that combine multiple types
    };
    
    /*!
     * @brief Built-in style categories for quick access
     */
    enum class DUCKX_API BuiltInStyleCategory
    {
        HEADING,     //!< Heading styles (Heading 1-6)
        BODY_TEXT,   //!< Body text variations
        LIST,        //!< List and numbering styles
        TABLE,       //!< Table formatting styles
        TECHNICAL    //!< Technical document styles (code, quote, etc.)
    };
    
    // ============================================================================
    // Style Property Definitions
    // ============================================================================
    
    /*!
     * @brief Container for paragraph-level style properties
     */
    struct DUCKX_API ParagraphStyleProperties
    {
        absl::optional<Alignment> alignment;
        absl::optional<double> space_before_pts = absl::nullopt;
        absl::optional<double> space_after_pts = absl::nullopt;
        absl::optional<double> line_spacing = absl::nullopt;
        absl::optional<double> left_indent_pts = absl::nullopt;
        absl::optional<double> right_indent_pts = absl::nullopt;
        absl::optional<double> first_line_indent_pts = absl::nullopt;
        absl::optional<ListType> list_type = absl::nullopt;
        absl::optional<int> list_level = absl::nullopt;
        
        ParagraphStyleProperties() = default;
    };
    
    /*!
     * @brief Container for character-level style properties
     */
    struct DUCKX_API CharacterStyleProperties
    {
        absl::optional<std::string> font_name;
        absl::optional<double> font_size_pts = absl::nullopt;
        absl::optional<std::string> font_color_hex;
        absl::optional<HighlightColor> highlight_color = absl::nullopt;
        absl::optional<formatting_flag> formatting_flags = absl::nullopt;
        
        CharacterStyleProperties() = default;
    };
    
    /*!
     * @brief Container for table-level style properties
     */
    struct DUCKX_API TableStyleProperties
    {
        absl::optional<std::string> border_style;
        absl::optional<double> border_width_pts = absl::nullopt;
        absl::optional<std::string> border_color_hex;
        absl::optional<double> cell_padding_pts = absl::nullopt;
        absl::optional<double> table_width_pts = absl::nullopt;
        absl::optional<std::string> table_alignment;
        
        TableStyleProperties() = default;
    };
    
    // ============================================================================
    // Style Class
    // ============================================================================
    
    /*!
     * @brief Represents a document style with properties and metadata
     * 
     * A Style encapsulates formatting properties that can be applied to document
     * elements. Supports inheritance, custom properties, and Result<T> operations.
     */
    class DUCKX_API Style
    {
    public:
        /*!
         * @brief Create a new style
         * @param name Unique style name
         * @param type Type of style (paragraph, character, etc.)
         */
        Style(const std::string& name, StyleType type);
        
        // Copy and move operations
        Style(const Style& other) = default;
        Style& operator=(const Style& other) = default;
        Style(Style&& other) noexcept = default;
        Style& operator=(Style&& other) noexcept = default;
        
        // Basic properties
        const std::string& name() const { return m_name; }
        StyleType type() const { return m_type; }
        bool is_built_in() const { return m_is_built_in; }
        bool is_custom() const { return !m_is_built_in; }
        
        // Style inheritance
        Result<void> set_base_style_safe(const std::string& base_style_name);
        const absl::optional<std::string>& base_style() const { return m_base_style; }
        
        // Paragraph properties
        Result<void> set_paragraph_properties_safe(const ParagraphStyleProperties& props);
        const ParagraphStyleProperties& paragraph_properties() const { return m_paragraph_props; }
        
        // Character properties  
        Result<void> set_character_properties_safe(const CharacterStyleProperties& props);
        const CharacterStyleProperties& character_properties() const { return m_character_props; }
        
        // Table properties
        Result<void> set_table_properties_safe(const TableStyleProperties& props);
        const TableStyleProperties& table_properties() const { return m_table_props; }
        
        // Convenience methods for common properties
        Result<void> set_font_safe(const std::string& font_name, double size_pts);
        Result<void> set_color_safe(const std::string& color_hex);
        Result<void> set_alignment_safe(Alignment alignment);
        Result<void> set_spacing_safe(double before_pts, double after_pts);
        
        // Style validation
        Result<void> validate_safe() const;
        
        // XML generation for DOCX
        Result<std::string> to_xml_safe() const;
        
    private:
        std::string m_name;
        StyleType m_type;
        bool m_is_built_in = false;
        absl::optional<std::string> m_base_style;
        
        ParagraphStyleProperties m_paragraph_props;
        CharacterStyleProperties m_character_props;
        TableStyleProperties m_table_props;
        
        friend class StyleManager;
    };
    
    // ============================================================================
    // StyleManager Class
    // ============================================================================
    
    /*!
     * @brief Central manager for document styles and style operations
     * 
     * StyleManager provides a comprehensive style management system with support
     * for creating, managing, and applying styles to document elements. Includes
     * built-in style libraries and inheritance support.
     */
    class DUCKX_API StyleManager
    {
    public:
        StyleManager();
        ~StyleManager() = default;
        
        // Disable copy operations to prevent style registry corruption
        StyleManager(const StyleManager&) = delete;
        StyleManager& operator=(const StyleManager&) = delete;
        
        // Enable move operations
        StyleManager(StyleManager&&) noexcept = default;
        StyleManager& operator=(StyleManager&&) noexcept = default;
        
        // ---- Style Creation and Management ----
        
        /*!
         * @brief Create a new paragraph style
         * @param name Unique style name
         * @return Result containing reference to created style or error
         */
        Result<Style*> create_paragraph_style_safe(const std::string& name);
        
        /*!
         * @brief Create a new character style  
         * @param name Unique style name
         * @return Result containing reference to created style or error
         */
        Result<Style*> create_character_style_safe(const std::string& name);
        
        /*!
         * @brief Create a new table style
         * @param name Unique style name
         * @return Result containing reference to created style or error
         */
        Result<Style*> create_table_style_safe(const std::string& name);
        
        /*!
         * @brief Create a new mixed style (supports both paragraph and character properties)
         * @param name Unique style name
         * @return Result containing reference to created style or error
         */
        Result<Style*> create_mixed_style_safe(const std::string& name);
        
        /*!
         * @brief Get an existing style by name
         * @param name Style name to retrieve
         * @return Result containing pointer to style or error if not found
         */
        Result<Style*> get_style_safe(const std::string& name);
        
        /*!
         * @brief Get an existing style by name (const version)
         * @param name Style name to retrieve  
         * @return Result containing const pointer to style or error if not found
         */
        Result<const Style*> get_style_safe(const std::string& name) const;
        
        /*!
         * @brief Remove a style from the manager
         * @param name Style name to remove
         * @return Result indicating success or error
         */
        Result<void> remove_style_safe(const std::string& name);
        
        /*!
         * @brief Check if a style exists
         * @param name Style name to check
         * @return true if style exists, false otherwise
         */
        bool has_style(const std::string& name) const;
        
        // ---- Built-in Style Management ----
        
        /*!
         * @brief Load built-in styles from a specific category
         * @param category Category of built-in styles to load
         * @return Result indicating success or error
         */
        Result<void> load_built_in_styles_safe(BuiltInStyleCategory category);
        
        /*!
         * @brief Load all available built-in styles
         * @return Result indicating success or error
         */
        Result<void> load_all_built_in_styles_safe();
        
        /*!
         * @brief Get list of available built-in style names
         * @param category Optional category filter
         * @return Vector of built-in style names
         */
        std::vector<std::string> get_built_in_style_names(
            absl::optional<BuiltInStyleCategory> category = absl::nullopt) const;
        
        // ---- Style Application ----
        
        /*!
         * @brief Apply a style to a document element
         * @param element Target element to apply style to
         * @param style_name Name of style to apply
         * @return Result indicating success or error
         */
        Result<void> apply_style_safe(DocxElement& element, const std::string& style_name);
        
        /*!
         * @brief Apply a style to a paragraph
         * @param paragraph Target paragraph
         * @param style_name Name of style to apply
         * @return Result indicating success or error
         */
        Result<void> apply_paragraph_style_safe(Paragraph& paragraph, const std::string& style_name);
        
        /*!
         * @brief Apply a style to a text run
         * @param run Target text run
         * @param style_name Name of style to apply  
         * @return Result indicating success or error
         */
        Result<void> apply_character_style_safe(Run& run, const std::string& style_name);
        
        /*!
         * @brief Apply a style to a table
         * @param table Target table
         * @param style_name Name of style to apply
         * @return Result indicating success or error
         */
        Result<void> apply_table_style_safe(Table& table, const std::string& style_name);
        
        // ---- Direct Property Application ----
        
        /*!
         * @brief Apply paragraph properties directly to a paragraph element
         * @param paragraph Target paragraph to apply properties to
         * @param props Paragraph properties to apply
         * @return Result indicating success or error
         */
        Result<void> apply_paragraph_properties_safe(Paragraph& paragraph, const ParagraphStyleProperties& props);
        
        /*!
         * @brief Apply character properties directly to a run element
         * @param run Target run to apply properties to
         * @param props Character properties to apply
         * @return Result indicating success or error
         */
        Result<void> apply_character_properties_safe(Run& run, const CharacterStyleProperties& props);
        
        /*!
         * @brief Apply table properties directly to a table element
         * @param table Target table to apply properties to
         * @param props Table properties to apply
         * @return Result indicating success or error
         */
        Result<void> apply_table_properties_safe(Table& table, const TableStyleProperties& props);
        
        // ---- Style Enumeration and Query ----
        
        /*!
         * @brief Get all registered style names
         * @return Vector of all style names
         */
        std::vector<std::string> get_all_style_names() const;
        
        /*!
         * @brief Get style names filtered by type
         * @param type Style type filter
         * @return Vector of style names matching the type
         */
        std::vector<std::string> get_style_names_by_type(StyleType type) const;
        
        /*!
         * @brief Get count of registered styles
         * @return Number of styles in the manager
         */
        size_t style_count() const { return m_styles.size(); }
        
        // ---- Style Reading and Extraction ----
        
        /*!
         * @brief Read paragraph properties from an element
         * @param element Target paragraph element to read from
         * @return Result containing paragraph style properties or error
         */
        Result<ParagraphStyleProperties> read_paragraph_properties_safe(const Paragraph& element) const;
        
        /*!
         * @brief Read character properties from an element
         * @param element Target run element to read from
         * @return Result containing character style properties or error
         */
        Result<CharacterStyleProperties> read_character_properties_safe(const Run& element) const;
        
        /*!
         * @brief Read table properties from an element
         * @param element Target table element to read from
         * @return Result containing table style properties or error
         */
        Result<TableStyleProperties> read_table_properties_safe(const Table& element) const;
        
        /*!
         * @brief Extract complete style definition from document element
         * @param element Source element to extract style from
         * @param style_name Name for the extracted style
         * @return Result containing pointer to created style or error
         */
        Result<Style*> extract_style_from_element_safe(const DocxElement& element, const std::string& style_name);
        
        /*!
         * @brief Read all effective properties of a paragraph (including inheritance)
         * @param paragraph Source paragraph
         * @return Result containing resolved paragraph properties or error
         */
        Result<ParagraphStyleProperties> get_effective_paragraph_properties_safe(const Paragraph& paragraph) const;
        
        /*!
         * @brief Read all effective properties of a run (including inheritance)
         * @param run Source run
         * @return Result containing resolved character properties or error
         */
        Result<CharacterStyleProperties> get_effective_character_properties_safe(const Run& run) const;
        
        /*!
         * @brief Read all effective properties of a table (including inheritance)
         * @param table Source table
         * @return Result containing resolved table properties or error
         */
        Result<TableStyleProperties> get_effective_table_properties_safe(const Table& table) const;
        
        /*!
         * @brief Compare two styles for differences
         * @param style1_name Name of first style to compare
         * @param style2_name Name of second style to compare
         * @return Result containing comparison report or error
         */
        Result<std::string> compare_styles_safe(const std::string& style1_name, const std::string& style2_name) const;
        
        // ---- Style Export and Import ----
        
        /*!
         * @brief Generate XML for all styles (for styles.xml in DOCX)
         * @return Result containing XML string or error
         */
        Result<std::string> generate_styles_xml_safe() const;
        
        /*!
         * @brief Clear all styles from the manager
         * @return Result indicating success or error
         */
        Result<void> clear_all_styles_safe();
        
        /*!
         * @brief Validate all styles in the manager
         * @return Result indicating success or error with details
         */
        Result<void> validate_all_styles_safe() const;
        
    private:
        // Style storage
        std::map<std::string, std::unique_ptr<Style>> m_styles;
        std::unordered_set<std::string> m_built_in_loaded_categories;
        
        // Helper methods
        Result<void> create_built_in_heading_styles_safe();
        Result<void> create_built_in_body_text_styles_safe();
        Result<void> create_built_in_list_styles_safe();
        Result<void> create_built_in_table_styles_safe();
        Result<void> create_built_in_technical_styles_safe();
        
        Result<Style*> create_style_internal_safe(const std::string& name, StyleType type);
        Result<void> validate_style_name_safe(const std::string& name) const;
        
        // Style reading helper methods
        Result<ParagraphStyleProperties> read_paragraph_properties_from_xml_safe(const pugi::xml_node& ppr_node) const;
        Result<CharacterStyleProperties> read_character_properties_from_xml_safe(const pugi::xml_node& rpr_node) const;
        Result<TableStyleProperties> read_table_properties_from_xml_safe(const pugi::xml_node& tblpr_node) const;
        Result<ParagraphStyleProperties> resolve_paragraph_inheritance_safe(const ParagraphStyleProperties& base_props, const std::string& style_name) const;
        Result<CharacterStyleProperties> resolve_character_inheritance_safe(const CharacterStyleProperties& base_props, const std::string& style_name) const;
        Result<TableStyleProperties> resolve_table_inheritance_safe(const TableStyleProperties& base_props, const std::string& style_name) const;
    };
    
} // namespace duckx