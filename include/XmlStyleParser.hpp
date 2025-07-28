/*!
 * @file XmlStyleParser.hpp
 * @brief XML-based style definition parser for DuckX-PlusPlus
 * 
 * Provides functionality to parse user-friendly XML style definitions
 * and convert them into internal Style objects. Supports comprehensive
 * style properties including paragraph, character, and table formatting,
 * plus style inheritance and validation.
 * 
 * @date 2025.07
 */
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "Error.hpp"
#include "StyleManager.hpp"
#include "duckx_export.h"
#include "pugixml.hpp"

namespace duckx
{
    // Forward declarations
    class Style;
    struct ParagraphStyleProperties;
    struct CharacterStyleProperties;
    struct TableStyleProperties;
    
    // ============================================================================
    // Style Set Definition
    // ============================================================================
    
    /*!
     * @brief Represents a collection of related styles
     * 
     * StyleSet allows organizing related styles into named collections
     * for easy management and application to documents.
     */
    struct DUCKX_API StyleSet
    {
        std::string name;                           //!< Name of the style set
        std::string description;                    //!< Optional description
        std::vector<std::string> included_styles;   //!< Names of styles in this set
        
        StyleSet() = default;
        StyleSet(const std::string& set_name) : name(set_name) {}
    };
    
    // ============================================================================
    // Unit Conversion Utilities
    // ============================================================================
    
    /*!
     * @brief Utility functions for parsing and formatting style values with units
     */
    namespace StyleUnits
    {
        /*!
         * @brief Parse a value string with unit suffix (e.g., "12pt", "1.5in")
         * @param value_str Value string with unit (e.g., "12pt", "24px", "1.5in")
         * @return Parsed value in points, or error if invalid format
         */
        DUCKX_API Result<double> parse_value_with_unit_safe(const std::string& value_str);
        
        /*!
         * @brief Format a numeric value with unit suffix
         * @param value Numeric value in points
         * @param unit Unit suffix to append (e.g., "pt", "px", "in")
         * @return Formatted string with unit
         */
        DUCKX_API std::string format_value_with_unit(double value, const std::string& unit = "pt");
        
        /*!
         * @brief Parse percentage value (e.g., "100%", "150%")
         * @param percent_str Percentage string
         * @return Parsed percentage as decimal (1.0 = 100%), or error if invalid
         */
        DUCKX_API Result<double> parse_percentage_safe(const std::string& percent_str);
        
        /*!
         * @brief Parse color value (hex format like "#FF0000" or named colors)  
         * @param color_str Color string in hex format or named color
         * @return Validated hex color string without #, or error if invalid
         */
        DUCKX_API Result<std::string> parse_color_safe(const std::string& color_str);
    }
    
    // ============================================================================
    // XmlStyleParser Class
    // ============================================================================
    
    /*!
     * @brief Parser for XML-based style definitions
     * 
     * XmlStyleParser provides functionality to load and parse user-friendly
     * XML style definitions, converting them into internal Style objects
     * that can be used with StyleManager. Supports validation, inheritance,
     * and comprehensive error reporting.
     */
    class DUCKX_API XmlStyleParser
    {
    public:
        XmlStyleParser() = default;
        ~XmlStyleParser() = default;
        
        // Disable copy operations to prevent parsing state corruption
        XmlStyleParser(const XmlStyleParser&) = delete;
        XmlStyleParser& operator=(const XmlStyleParser&) = delete;
        
        // Enable move operations
        XmlStyleParser(XmlStyleParser&&) noexcept = default;
        XmlStyleParser& operator=(XmlStyleParser&&) noexcept = default;
        
        // ---- Primary Parsing Interface ----
        
        /*!
         * @brief Load styles from XML file
         * @param filepath Path to XML style definition file
         * @return Result containing vector of parsed styles or error
         */
        Result<std::vector<std::unique_ptr<Style>>> load_styles_from_file_safe(const std::string& filepath);
        
        /*!
         * @brief Load styles from XML string content
         * @param xml_content XML style definition as string
         * @return Result containing vector of parsed styles or error
         */
        Result<std::vector<std::unique_ptr<Style>>> load_styles_from_string_safe(const std::string& xml_content);
        
        /*!
         * @brief Load style sets from XML file
         * @param filepath Path to XML style definition file
         * @return Result containing vector of parsed style sets or error
         */
        Result<std::vector<StyleSet>> load_style_sets_from_file_safe(const std::string& filepath);
        
        /*!
         * @brief Load style sets from XML string content
         * @param xml_content XML style definition as string
         * @return Result containing vector of parsed style sets or error
         */
        Result<std::vector<StyleSet>> load_style_sets_from_string_safe(const std::string& xml_content);
        
        // ---- Validation and Utility ----
        
        /*!
         * @brief Validate XML style definition format
         * @param doc Parsed XML document to validate
         * @return Result indicating validation success or specific errors
         */
        Result<void> validate_style_xml_safe(const pugi::xml_document& doc);
        
        /*!
         * @brief Get supported XML schema version
         * @return Current supported XML schema version string
         */
        static std::string get_supported_schema_version() { return "1.0"; }
        
        /*!
         * @brief Get XML namespace URI
         * @return XML namespace URI for style definitions
         */
        static std::string get_xml_namespace() { return "http://duckx.org/styles"; }
        
    private:
        // ---- Core Parsing Methods ----
        
        /*!
         * @brief Parse XML document and extract styles and style sets
         * @param doc Parsed XML document
         * @param styles_out Output vector for parsed styles
         * @param style_sets_out Output vector for parsed style sets
         * @return Result indicating parsing success or error
         */
        Result<void> parse_xml_document_safe(const pugi::xml_document& doc,
                                           std::vector<std::unique_ptr<Style>>& styles_out,
                                           std::vector<StyleSet>& style_sets_out);
        
        /*!
         * @brief Parse a single style definition node
         * @param style_node XML node containing style definition
         * @return Result containing parsed style or error
         */
        Result<std::unique_ptr<Style>> parse_style_node_safe(const pugi::xml_node& style_node);
        
        /*!
         * @brief Parse a single style set definition node
         * @param set_node XML node containing style set definition
         * @return Result containing parsed style set or error
         */
        Result<StyleSet> parse_style_set_node_safe(const pugi::xml_node& set_node);
        
        // ---- Property Parsing Methods ----
        
        /*!
         * @brief Parse paragraph properties from XML node
         * @param para_node XML node containing paragraph properties
         * @return Result containing parsed paragraph properties or error
         */
        Result<ParagraphStyleProperties> parse_paragraph_properties_safe(const pugi::xml_node& para_node);
        
        /*!
         * @brief Parse character properties from XML node
         * @param char_node XML node containing character properties
         * @return Result containing parsed character properties or error
         */
        Result<CharacterStyleProperties> parse_character_properties_safe(const pugi::xml_node& char_node);
        
        /*!
         * @brief Parse table properties from XML node
         * @param table_node XML node containing table properties
         * @return Result containing parsed table properties or error
         */
        Result<TableStyleProperties> parse_table_properties_safe(const pugi::xml_node& table_node);
        
        // ---- Helper Methods ----
        
        /*!
         * @brief Parse alignment value from string
         * @param alignment_str Alignment string (left, center, right, justify)
         * @return Result containing parsed alignment or error
         */
        Result<Alignment> parse_alignment_safe(const std::string& alignment_str);
        
        /*!
         * @brief Parse list type from string
         * @param list_type_str List type string (bullet, numbered, etc.)
         * @return Result containing parsed list type or error
         */
        Result<ListType> parse_list_type_safe(const std::string& list_type_str);
        
        /*!
         * @brief Parse highlight color from string
         * @param highlight_str Highlight color string
         * @return Result containing parsed highlight color or error
         */
        Result<HighlightColor> parse_highlight_color_safe(const std::string& highlight_str);
        
        /*!
         * @brief Parse formatting flags from XML node
         * @param format_node XML node containing format attributes
         * @return Result containing parsed formatting flags or error
         */
        Result<formatting_flag> parse_formatting_flags_safe(const pugi::xml_node& format_node);
        
        /*!
         * @brief Validate style type string
         * @param type_str Style type string (paragraph, character, table, mixed)
         * @return Result containing parsed style type or error
         */
        Result<StyleType> parse_style_type_safe(const std::string& type_str);
        
        /*!
         * @brief Load and parse XML document from file or string
         * @param source Source content (file path or XML string)
         * @param is_file_path True if source is file path, false if XML content
         * @return Result containing parsed XML document or error
         */
        Result<pugi::xml_document> load_xml_document_safe(const std::string& source, bool is_file_path);
    };
    
} // namespace duckx