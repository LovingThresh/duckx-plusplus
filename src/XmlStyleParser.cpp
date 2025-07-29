/*!
 * @file XmlStyleParser.cpp
 * @brief Implementation of XML-based style definition parser
 * 
 * Provides parsing functionality for user-friendly XML style definitions,
 * converting them to internal Style objects with comprehensive validation
 * and error handling.
 */

#include "XmlStyleParser.hpp"
#include "constants.hpp"

#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"

#include <fstream>
#include <cctype>

namespace duckx
{
    // ============================================================================
    // StyleUnits Namespace Implementation
    // ============================================================================
    
    namespace StyleUnits
    {
        Result<double> parse_value_with_unit_safe(const std::string& value_str)
        {
            if (value_str.empty()) {
                return Result<double>(errors::invalid_argument("value_str", 
                    "Value string cannot be empty", DUCKX_ERROR_CONTEXT()));
            }
            
            // Find the start of the unit suffix
            size_t unit_start = value_str.length();
            for (size_t i = 0; i < value_str.length(); ++i) {
                if (!std::isdigit(value_str[i]) && value_str[i] != '.' && value_str[i] != '-') {
                    unit_start = i;
                    break;
                }
            }
            
            if (unit_start == 0) {
                return Result<double>(errors::invalid_argument("value_str",
                    absl::StrFormat("Invalid numeric value: '%s'", value_str),
                    DUCKX_ERROR_CONTEXT()));
            }
            
            // Extract numeric part
            std::string numeric_part = value_str.substr(0, unit_start);
            std::string unit_part = value_str.substr(unit_start);
            
            // Parse numeric value
            double numeric_value;
            if (!absl::SimpleAtod(numeric_part, &numeric_value)) {
                return Result<double>(errors::invalid_argument("value_str",
                    absl::StrFormat("Cannot parse numeric value: '%s'", numeric_part),
                    DUCKX_ERROR_CONTEXT()));
            }
            
            // Convert to points based on unit
            if (unit_part.empty() || unit_part == "pt") {
                return Result<double>{numeric_value}; // Already in points
            } else if (unit_part == "px") {
                return Result<double>{numeric_value * 0.75}; // 1px = 0.75pt
            } else if (unit_part == "in") {
                return Result<double>{numeric_value * 72.0}; // 1in = 72pt
            } else if (unit_part == "cm") {
                return Result<double>{numeric_value * 28.35}; // 1cm ≈ 28.35pt
            } else if (unit_part == "mm") {
                return Result<double>{numeric_value * 2.835}; // 1mm ≈ 2.835pt
            } else {
                return Result<double>(errors::invalid_argument("unit",
                    absl::StrFormat("Unsupported unit: '%s'", unit_part),
                    DUCKX_ERROR_CONTEXT()));
            }
        }
        
        std::string format_value_with_unit(double value, const std::string& unit)
        {
            return absl::StrFormat("%.1f%s", value, unit);
        }
        
        Result<double> parse_percentage_safe(const std::string& percent_str)
        {
            if (percent_str.empty() || percent_str.back() != '%') {
                return Result<double>(errors::invalid_argument("percent_str",
                    absl::StrFormat("Invalid percentage format: '%s'", percent_str),
                    DUCKX_ERROR_CONTEXT()));
            }
            
            std::string numeric_part = percent_str.substr(0, percent_str.length() - 1);
            double numeric_value;
            if (!absl::SimpleAtod(numeric_part, &numeric_value)) {
                return Result<double>(errors::invalid_argument("percent_str",
                    absl::StrFormat("Cannot parse percentage value: '%s'", numeric_part),
                    DUCKX_ERROR_CONTEXT()));
            }
            
            return Result<double>{numeric_value / 100.0}; // Convert to decimal
        }
        
        Result<std::string> parse_color_safe(const std::string& color_str)
        {
            if (color_str.empty()) {
                return Result<std::string>(errors::invalid_argument("color_str",
                    "Color string cannot be empty", DUCKX_ERROR_CONTEXT()));
            }
            
            std::string color_lower = absl::AsciiStrToLower(color_str);
            
            // Handle named colors
            if (color_lower == "black") return Result<std::string>{"000000"};
            if (color_lower == "white") return Result<std::string>{"FFFFFF"};
            if (color_lower == "red") return Result<std::string>{"FF0000"};
            if (color_lower == "green") return Result<std::string>{"008000"};
            if (color_lower == "blue") return Result<std::string>{"0000FF"};
            if (color_lower == "yellow") return Result<std::string>{"FFFF00"};
            if (color_lower == "cyan") return Result<std::string>{"00FFFF"};
            if (color_lower == "magenta") return Result<std::string>{"FF00FF"};
            
            // Handle hex colors
            std::string hex_color = color_str;
            if (hex_color.front() == '#') {
                hex_color = hex_color.substr(1);
            }
            
            if (hex_color.length() != 6) {
                return Result<std::string>(errors::invalid_argument("color_str",
                    absl::StrFormat("Invalid hex color format: '%s'", color_str),
                    DUCKX_ERROR_CONTEXT()));
            }
            
            // Validate hex characters
            for (char c : hex_color) {
                if (!std::isxdigit(c)) {
                    return Result<std::string>(errors::invalid_argument("color_str",
                        absl::StrFormat("Invalid hex character in color: '%s'", color_str),
                        DUCKX_ERROR_CONTEXT()));
                }
            }
            
            return Result<std::string>{absl::AsciiStrToUpper(hex_color)};
        }
    }
    
    // ============================================================================
    // XmlStyleParser Implementation
    // ============================================================================
    
    Result<std::vector<std::unique_ptr<Style>>> XmlStyleParser::load_styles_from_file_safe(const std::string& filepath)
    {
        auto doc_result = load_xml_document_safe(filepath, true);
        if (!doc_result.ok()) {
            return Result<std::vector<std::unique_ptr<Style>>>(doc_result.error());
        }
        
        auto validation_result = validate_style_xml_safe(doc_result.value());
        if (!validation_result.ok()) {
            return Result<std::vector<std::unique_ptr<Style>>>(validation_result.error());
        }
        
        std::vector<std::unique_ptr<Style>> styles;
        std::vector<StyleSet> style_sets; // Will be used later
        
        auto parse_result = parse_xml_document_safe(doc_result.value(), styles, style_sets);
        if (!parse_result.ok()) {
            return Result<std::vector<std::unique_ptr<Style>>>(parse_result.error());
        }
        
        return Result<std::vector<std::unique_ptr<Style>>>{std::move(styles)};
    }
    
    Result<std::vector<std::unique_ptr<Style>>> XmlStyleParser::load_styles_from_string_safe(const std::string& xml_content)
    {
        auto doc_result = load_xml_document_safe(xml_content, false);
        if (!doc_result.ok()) {
            return Result<std::vector<std::unique_ptr<Style>>>(doc_result.error());
        }
        
        auto validation_result = validate_style_xml_safe(doc_result.value());
        if (!validation_result.ok()) {
            return Result<std::vector<std::unique_ptr<Style>>>(validation_result.error());
        }
        
        std::vector<std::unique_ptr<Style>> styles;
        std::vector<StyleSet> style_sets; // Will be used later
        
        auto parse_result = parse_xml_document_safe(doc_result.value(), styles, style_sets);
        if (!parse_result.ok()) {
            return Result<std::vector<std::unique_ptr<Style>>>(parse_result.error());
        }
        
        return Result<std::vector<std::unique_ptr<Style>>>{std::move(styles)};
    }
    
    Result<std::vector<StyleSet>> XmlStyleParser::load_style_sets_from_file_safe(const std::string& filepath)
    {
        auto doc_result = load_xml_document_safe(filepath, true);
        if (!doc_result.ok()) {
            return Result<std::vector<StyleSet>>(doc_result.error());
        }
        
        auto validation_result = validate_style_xml_safe(doc_result.value());
        if (!validation_result.ok()) {
            return Result<std::vector<StyleSet>>(validation_result.error());
        }
        
        std::vector<std::unique_ptr<Style>> styles; // Will be ignored
        std::vector<StyleSet> style_sets;
        
        auto parse_result = parse_xml_document_safe(doc_result.value(), styles, style_sets);
        if (!parse_result.ok()) {
            return Result<std::vector<StyleSet>>(parse_result.error());
        }
        
        return Result<std::vector<StyleSet>>{std::move(style_sets)};
    }
    
    Result<std::vector<StyleSet>> XmlStyleParser::load_style_sets_from_string_safe(const std::string& xml_content)
    {
        auto doc_result = load_xml_document_safe(xml_content, false);
        if (!doc_result.ok()) {
            return Result<std::vector<StyleSet>>(doc_result.error());
        }
        
        auto validation_result = validate_style_xml_safe(doc_result.value());
        if (!validation_result.ok()) {
            return Result<std::vector<StyleSet>>(validation_result.error());
        }
        
        std::vector<std::unique_ptr<Style>> styles; // Will be ignored
        std::vector<StyleSet> style_sets;
        
        auto parse_result = parse_xml_document_safe(doc_result.value(), styles, style_sets);
        if (!parse_result.ok()) {
            return Result<std::vector<StyleSet>>(parse_result.error());
        }
        
        return Result<std::vector<StyleSet>>{std::move(style_sets)};
    }
    
    Result<void> XmlStyleParser::validate_style_xml_safe(const pugi::xml_document& doc)
    {
        pugi::xml_node root = doc.first_child();
        if (!root || std::string(root.name()) != "StyleSheet") {
            return Result<void>(errors::xml_parse_error(
                "Root element must be 'StyleSheet'", DUCKX_ERROR_CONTEXT()));
        }
        
        // Check for required namespace
        pugi::xml_attribute xmlns_attr = root.attribute("xmlns");
        if (!xmlns_attr || std::string(xmlns_attr.value()) != get_xml_namespace()) {
            return Result<void>(errors::xml_parse_error(
                absl::StrFormat("Missing or incorrect xmlns attribute. Expected: %s", 
                    get_xml_namespace()), DUCKX_ERROR_CONTEXT()));
        }
        
        // Check version compatibility
        pugi::xml_attribute version_attr = root.attribute("version");
        if (!version_attr) {
            return Result<void>(errors::xml_parse_error(
                "Missing 'version' attribute in StyleSheet", DUCKX_ERROR_CONTEXT()));
        }
        
        std::string version = version_attr.value();
        if (version != get_supported_schema_version()) {
            return Result<void>(errors::xml_parse_error(
                absl::StrFormat("Unsupported schema version: %s. Supported: %s",
                    version, get_supported_schema_version()), DUCKX_ERROR_CONTEXT()));
        }
        
        return Result<void>{};
    }
    
    // ---- Private Implementation Methods ----
    
    Result<pugi::xml_document> XmlStyleParser::load_xml_document_safe(const std::string& source, bool is_file_path)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result;
        
        if (is_file_path) {
            result = doc.load_file(source.c_str());
            if (!result) {
                return Result<pugi::xml_document>(errors::file_not_found(
                    absl::StrFormat("Failed to load XML file '%s': %s", 
                        source, result.description()), DUCKX_ERROR_CONTEXT()));
            }
        } else {
            result = doc.load_string(source.c_str());
            if (!result) {
                return Result<pugi::xml_document>(errors::xml_parse_error(
                    absl::StrFormat("Failed to parse XML content: %s", 
                        result.description()), DUCKX_ERROR_CONTEXT()));
            }
        }
        
        return Result<pugi::xml_document>{std::move(doc)};
    }
    
    Result<void> XmlStyleParser::parse_xml_document_safe(const pugi::xml_document& doc,
                                                        std::vector<std::unique_ptr<Style>>& styles_out,
                                                        std::vector<StyleSet>& style_sets_out)
    {
        pugi::xml_node root = doc.first_child();
        
        // Parse individual styles
        for (pugi::xml_node style_node = root.child("Style"); style_node; style_node = style_node.next_sibling("Style")) {
            auto style_result = parse_style_node_safe(style_node);
            if (!style_result.ok()) {
                return Result<void>(style_result.error());
            }
            styles_out.push_back(std::move(style_result.value()));
        }
        
        // Parse style sets
        for (pugi::xml_node set_node = root.child("StyleSet"); set_node; set_node = set_node.next_sibling("StyleSet")) {
            auto set_result = parse_style_set_node_safe(set_node);
            if (!set_result.ok()) {
                return Result<void>(set_result.error());
            }
            style_sets_out.push_back(std::move(set_result.value()));
        }
        
        return Result<void>{};
    }
    
    Result<std::unique_ptr<Style>> XmlStyleParser::parse_style_node_safe(const pugi::xml_node& style_node)
    {
        // Get style name
        pugi::xml_attribute name_attr = style_node.attribute("name");
        if (!name_attr) {
            return Result<std::unique_ptr<Style>>(errors::xml_parse_error(
                "Style node missing 'name' attribute", DUCKX_ERROR_CONTEXT()));
        }
        std::string style_name = name_attr.value();
        
        // Get style type
        pugi::xml_attribute type_attr = style_node.attribute("type");
        if (!type_attr) {
            return Result<std::unique_ptr<Style>>(errors::xml_parse_error(
                absl::StrFormat("Style '%s' missing 'type' attribute", style_name), 
                DUCKX_ERROR_CONTEXT()));
        }
        
        auto type_result = parse_style_type_safe(type_attr.value());
        if (!type_result.ok()) {
            return Result<std::unique_ptr<Style>>(type_result.error());
        }
        
        // Create the style
        auto style = std::make_unique<Style>(style_name, type_result.value());
        
        // Set base style if specified
        pugi::xml_attribute base_attr = style_node.attribute("base");
        if (base_attr) {
            auto base_result = style->set_base_style_safe(base_attr.value());
            if (!base_result.ok()) {
                return Result<std::unique_ptr<Style>>(base_result.error());
            }
        }
        
        // Parse paragraph properties if present
        pugi::xml_node paragraph_node = style_node.child("Paragraph");
        if (paragraph_node) {
            auto para_result = parse_paragraph_properties_safe(paragraph_node);
            if (!para_result.ok()) {
                return Result<std::unique_ptr<Style>>(para_result.error());
            }
            
            auto set_para_result = style->set_paragraph_properties_safe(para_result.value());
            if (!set_para_result.ok()) {
                return Result<std::unique_ptr<Style>>(set_para_result.error());
            }
        }
        
        // Parse character properties if present
        pugi::xml_node character_node = style_node.child("Character");
        if (character_node) {
            auto char_result = parse_character_properties_safe(character_node);
            if (!char_result.ok()) {
                return Result<std::unique_ptr<Style>>(char_result.error());
            }
            
            auto set_char_result = style->set_character_properties_safe(char_result.value());
            if (!set_char_result.ok()) {
                return Result<std::unique_ptr<Style>>(set_char_result.error());
            }
        }
        
        // Parse table properties if present
        pugi::xml_node table_node = style_node.child("Table");
        if (table_node) {
            auto table_result = parse_table_properties_safe(table_node);
            if (!table_result.ok()) {
                return Result<std::unique_ptr<Style>>(table_result.error());
            }
            
            auto set_table_result = style->set_table_properties_safe(table_result.value());
            if (!set_table_result.ok()) {
                return Result<std::unique_ptr<Style>>(set_table_result.error());
            }
        }
        
        // Validate the completed style
        auto validation_result = style->validate_safe();
        if (!validation_result.ok()) {
            return Result<std::unique_ptr<Style>>(validation_result.error());
        }
        
        return Result<std::unique_ptr<Style>>{std::move(style)};
    }
    
    Result<StyleSet> XmlStyleParser::parse_style_set_node_safe(const pugi::xml_node& set_node)
    {
        // Get set name
        pugi::xml_attribute name_attr = set_node.attribute("name");
        if (!name_attr) {
            return Result<StyleSet>(errors::xml_parse_error(
                "StyleSet node missing 'name' attribute", DUCKX_ERROR_CONTEXT()));
        }
        
        StyleSet style_set(name_attr.value());
        
        // Get optional description
        pugi::xml_attribute desc_attr = set_node.attribute("description");
        if (desc_attr) {
            style_set.description = desc_attr.value();
        }
        
        // Parse included styles
        for (pugi::xml_node include_node = set_node.child("Include"); 
             include_node; 
             include_node = include_node.next_sibling("Include")) {
            
            std::string style_name = include_node.child_value();
            if (style_name.empty()) {
                return Result<StyleSet>(errors::xml_parse_error(
                    absl::StrFormat("StyleSet '%s' contains empty Include element", 
                        style_set.name), DUCKX_ERROR_CONTEXT()));
            }
            
            style_set.included_styles.push_back(style_name);
        }
        
        if (style_set.included_styles.empty()) {
            return Result<StyleSet>(errors::validation_failed("included_styles",
                absl::StrFormat("StyleSet '%s' must include at least one style", 
                    style_set.name), DUCKX_ERROR_CONTEXT()));
        }
        
        return Result<StyleSet>{std::move(style_set)};
    }
    
    Result<ParagraphStyleProperties> XmlStyleParser::parse_paragraph_properties_safe(const pugi::xml_node& para_node)
    {
        ParagraphStyleProperties props;
        
        // Parse alignment
        pugi::xml_node alignment_node = para_node.child("Alignment");
        if (alignment_node) {
            auto alignment_result = parse_alignment_safe(alignment_node.child_value());
            if (!alignment_result.ok()) {
                return Result<ParagraphStyleProperties>(alignment_result.error());
            }
            props.alignment = alignment_result.value();
        }
        
        // Parse space before
        pugi::xml_node space_before_node = para_node.child("SpaceBefore");
        if (space_before_node) {
            auto space_result = StyleUnits::parse_value_with_unit_safe(space_before_node.child_value());
            if (!space_result.ok()) {
                return Result<ParagraphStyleProperties>(space_result.error());
            }
            props.space_before_pts = space_result.value();
        }
        
        // Parse space after
        pugi::xml_node space_after_node = para_node.child("SpaceAfter");
        if (space_after_node) {
            auto space_result = StyleUnits::parse_value_with_unit_safe(space_after_node.child_value());
            if (!space_result.ok()) {
                return Result<ParagraphStyleProperties>(space_result.error());
            }
            props.space_after_pts = space_result.value();
        }
        
        // Parse line spacing
        pugi::xml_node line_spacing_node = para_node.child("LineSpacing");
        if (line_spacing_node) {
            std::string spacing_str = line_spacing_node.child_value();
            double spacing_value;
            if (!absl::SimpleAtod(spacing_str, &spacing_value)) {
                return Result<ParagraphStyleProperties>(errors::invalid_argument("LineSpacing",
                    absl::StrFormat("Invalid line spacing value: '%s'", spacing_str),
                    DUCKX_ERROR_CONTEXT()));
            }
            props.line_spacing = spacing_value;
        }
        
        // Parse indentation
        pugi::xml_node indentation_node = para_node.child("Indentation");
        if (indentation_node) {
            // Left indentation
            pugi::xml_attribute left_attr = indentation_node.attribute("left");
            if (left_attr) {
                auto left_result = StyleUnits::parse_value_with_unit_safe(left_attr.value());
                if (!left_result.ok()) {
                    return Result<ParagraphStyleProperties>(left_result.error());
                }
                props.left_indent_pts = left_result.value();
            }
            
            // Right indentation
            pugi::xml_attribute right_attr = indentation_node.attribute("right");
            if (right_attr) {
                auto right_result = StyleUnits::parse_value_with_unit_safe(right_attr.value());
                if (!right_result.ok()) {
                    return Result<ParagraphStyleProperties>(right_result.error());
                }
                props.right_indent_pts = right_result.value();
            }
            
            // First line indentation
            pugi::xml_attribute first_line_attr = indentation_node.attribute("firstLine");
            if (first_line_attr) {
                auto first_line_result = StyleUnits::parse_value_with_unit_safe(first_line_attr.value());
                if (!first_line_result.ok()) {
                    return Result<ParagraphStyleProperties>(first_line_result.error());
                }
                props.first_line_indent_pts = first_line_result.value();
            }
        }
        
        return Result<ParagraphStyleProperties>{props};
    }
    
    Result<CharacterStyleProperties> XmlStyleParser::parse_character_properties_safe(const pugi::xml_node& char_node)
    {
        CharacterStyleProperties props;
        
        // Parse font information
        pugi::xml_node font_node = char_node.child("Font");
        if (font_node) {
            // Font name
            pugi::xml_attribute name_attr = font_node.attribute("name");
            if (name_attr) {
                props.font_name = name_attr.value();
            }
            
            // Font size
            pugi::xml_attribute size_attr = font_node.attribute("size");
            if (size_attr) {
                auto size_result = StyleUnits::parse_value_with_unit_safe(size_attr.value());
                if (!size_result.ok()) {
                    return Result<CharacterStyleProperties>(size_result.error());
                }
                props.font_size_pts = size_result.value();
            }
        }
        
        // Parse color
        pugi::xml_node color_node = char_node.child("Color");
        if (color_node) {
            auto color_result = StyleUnits::parse_color_safe(color_node.child_value());
            if (!color_result.ok()) {
                return Result<CharacterStyleProperties>(color_result.error());
            }
            props.font_color_hex = color_result.value();
        }
        
        // Parse highlight
        pugi::xml_node highlight_node = char_node.child("Highlight");
        if (highlight_node) {
            std::string highlight_str = highlight_node.child_value();
            if (!highlight_str.empty()) {
                auto highlight_result = parse_highlight_color_safe(highlight_str);
                if (highlight_result.ok()) {
                    props.highlight_color = highlight_result.value();
                }
                // If parsing fails, we ignore the highlight (could log warning in production)
            }
        }
        
        // Parse formatting flags
        pugi::xml_node format_node = char_node.child("Format");
        if (format_node) {
            auto flags_result = parse_formatting_flags_safe(format_node);
            if (flags_result.ok() && flags_result.value() != none) {
                props.formatting_flags = flags_result.value();
            }
            // If parsing fails, we ignore the formatting flags (could log warning in production)
        }
        
        return Result<CharacterStyleProperties>{props};
    }
    
    Result<TableStyleProperties> XmlStyleParser::parse_table_properties_safe(const pugi::xml_node& table_node)
    {
        TableStyleProperties props;
        
        // Parse table width
        pugi::xml_node width_node = table_node.child("Width");
        if (width_node) {
            std::string width_str = width_node.child_value();
            if (width_str.back() == '%') {
                // Handle percentage width - convert to points later or store as-is
                // For now, just parse the numeric part (will need conversion logic)
                auto percent_result = StyleUnits::parse_percentage_safe(width_str);
                if (!percent_result.ok()) {
                    return Result<TableStyleProperties>(percent_result.error());
                }
                // Store as percentage * 400 (assuming 400pt as 100% width)
                props.table_width_pts = percent_result.value() * 400.0;
            } else {
                auto width_result = StyleUnits::parse_value_with_unit_safe(width_str);
                if (!width_result.ok()) {
                    return Result<TableStyleProperties>(width_result.error());
                }
                props.table_width_pts = width_result.value();
            }
        }
        
        // Parse table alignment
        pugi::xml_node alignment_node = table_node.child("Alignment");
        if (alignment_node) {
            props.table_alignment = alignment_node.child_value();
        }
        
        // Parse borders
        pugi::xml_node borders_node = table_node.child("Borders");
        if (borders_node) {
            // Border style
            pugi::xml_attribute style_attr = borders_node.attribute("style");
            if (style_attr) {
                props.border_style = style_attr.value();
            }
            
            // Border width
            pugi::xml_attribute width_attr = borders_node.attribute("width");
            if (width_attr) {
                auto width_result = StyleUnits::parse_value_with_unit_safe(width_attr.value());
                if (!width_result.ok()) {
                    return Result<TableStyleProperties>(width_result.error());
                }
                props.border_width_pts = width_result.value();
            }
            
            // Border color
            pugi::xml_attribute color_attr = borders_node.attribute("color");
            if (color_attr) {
                auto color_result = StyleUnits::parse_color_safe(color_attr.value());
                if (!color_result.ok()) {
                    return Result<TableStyleProperties>(color_result.error());
                }
                props.border_color_hex = color_result.value();
            }
        }
        
        // Parse cell padding
        pugi::xml_node padding_node = table_node.child("CellPadding");
        if (padding_node) {
            auto padding_result = StyleUnits::parse_value_with_unit_safe(padding_node.child_value());
            if (!padding_result.ok()) {
                return Result<TableStyleProperties>(padding_result.error());
            }
            props.cell_padding_pts = padding_result.value();
        }
        
        return Result<TableStyleProperties>{props};
    }
    
    Result<Alignment> XmlStyleParser::parse_alignment_safe(const std::string& alignment_str)
    {
        std::string align_lower = absl::AsciiStrToLower(alignment_str);
        
        if (align_lower == "left") return Result<Alignment>{Alignment::LEFT};
        if (align_lower == "center") return Result<Alignment>{Alignment::CENTER};
        if (align_lower == "right") return Result<Alignment>{Alignment::RIGHT};
        if (align_lower == "justify") return Result<Alignment>{Alignment::BOTH};
        
        return Result<Alignment>(errors::invalid_argument("alignment_str",
            absl::StrFormat("Invalid alignment value: '%s'", alignment_str),
            DUCKX_ERROR_CONTEXT()));
    }
    
    Result<StyleType> XmlStyleParser::parse_style_type_safe(const std::string& type_str)
    {
        std::string type_lower = absl::AsciiStrToLower(type_str);
        
        if (type_lower == "paragraph") return Result<StyleType>{StyleType::PARAGRAPH};
        if (type_lower == "character") return Result<StyleType>{StyleType::CHARACTER};
        if (type_lower == "table") return Result<StyleType>{StyleType::TABLE};
        if (type_lower == "numbering") return Result<StyleType>{StyleType::NUMBERING};
        if (type_lower == "mixed") return Result<StyleType>{StyleType::MIXED};
        
        return Result<StyleType>(errors::invalid_argument("type_str",
            absl::StrFormat("Invalid style type: '%s'", type_str),
            DUCKX_ERROR_CONTEXT()));
    }
    
    // Additional helper method placeholders - will implement as needed
    Result<ListType> XmlStyleParser::parse_list_type_safe(const std::string& list_type_str)
    {
        std::string type_lower = absl::AsciiStrToLower(list_type_str);
        
        if (type_lower == "bullet" || type_lower == "unordered") {
            return Result<ListType>{ListType::BULLET};
        }
        if (type_lower == "numbered" || type_lower == "ordered" || type_lower == "decimal" || type_lower == "number") {
            return Result<ListType>{ListType::NUMBER};
        }
        if (type_lower == "none") {
            return Result<ListType>{ListType::NONE};
        }
        
        return Result<ListType>(errors::invalid_argument("list_type_str",
            absl::StrFormat("Invalid list type: '%s'. Supported: none, bullet, number/numbered", 
                list_type_str), DUCKX_ERROR_CONTEXT()));
    }
    
    Result<HighlightColor> XmlStyleParser::parse_highlight_color_safe(const std::string& highlight_str)
    {
        if (highlight_str.empty()) {
            return Result<HighlightColor>(errors::invalid_argument("highlight_str",
                "Highlight color string cannot be empty", DUCKX_ERROR_CONTEXT()));
        }
        
        std::string color_lower = absl::AsciiStrToLower(highlight_str);
        
        if (color_lower == "yellow") {
            return Result<HighlightColor>{HighlightColor::YELLOW};
        }
        if (color_lower == "lightgray" || color_lower == "lightgrey" || color_lower == "light-gray") {
            return Result<HighlightColor>{HighlightColor::LIGHT_GRAY};
        }
        if (color_lower == "green") {
            return Result<HighlightColor>{HighlightColor::GREEN};
        }
        if (color_lower == "cyan") {
            return Result<HighlightColor>{HighlightColor::CYAN};
        }
        if (color_lower == "magenta") {
            return Result<HighlightColor>{HighlightColor::MAGENTA};
        }
        if (color_lower == "blue") {
            return Result<HighlightColor>{HighlightColor::BLUE};
        }
        if (color_lower == "red") {
            return Result<HighlightColor>{HighlightColor::RED};
        }
        if (color_lower == "darkblue" || color_lower == "dark-blue") {
            return Result<HighlightColor>{HighlightColor::DARK_BLUE};
        }
        if (color_lower == "darkcyan" || color_lower == "dark-cyan") {
            return Result<HighlightColor>{HighlightColor::DARK_CYAN};
        }
        if (color_lower == "darkgreen" || color_lower == "dark-green") {
            return Result<HighlightColor>{HighlightColor::DARK_GREEN};
        }
        if (color_lower == "darkmagenta" || color_lower == "dark-magenta") {
            return Result<HighlightColor>{HighlightColor::DARK_MAGENTA};
        }
        if (color_lower == "darkred" || color_lower == "dark-red") {
            return Result<HighlightColor>{HighlightColor::DARK_RED};
        }
        if (color_lower == "darkyellow" || color_lower == "dark-yellow") {
            return Result<HighlightColor>{HighlightColor::DARK_YELLOW};
        }
        if (color_lower == "white") {
            return Result<HighlightColor>{HighlightColor::WHITE};
        }
        if (color_lower == "black") {
            return Result<HighlightColor>{HighlightColor::BLACK};
        }
        
        return Result<HighlightColor>(errors::invalid_argument("highlight_str",
            absl::StrFormat("Invalid highlight color: '%s'. Supported colors: yellow, lightgray, green, cyan, magenta, blue, red, darkblue, darkcyan, darkgreen, darkmagenta, darkred, darkyellow, darkgray, black", 
                highlight_str), DUCKX_ERROR_CONTEXT()));
    }
    
    Result<formatting_flag> XmlStyleParser::parse_formatting_flags_safe(const pugi::xml_node& format_node)
    {
        if (!format_node) {
            return Result<formatting_flag>(errors::invalid_argument("format_node",
                "Format node cannot be null", DUCKX_ERROR_CONTEXT()));
        }
        
        formatting_flag flags = none;
        
        // Parse bold
        pugi::xml_attribute bold_attr = format_node.attribute("bold");
        if (bold_attr) {
            std::string bold_value = absl::AsciiStrToLower(bold_attr.value());
            if (bold_value == "true" || bold_value == "1" || bold_value == "yes") {
                flags |= bold;
            }
        }
        
        // Parse italic
        pugi::xml_attribute italic_attr = format_node.attribute("italic");
        if (italic_attr) {
            std::string italic_value = absl::AsciiStrToLower(italic_attr.value());
            if (italic_value == "true" || italic_value == "1" || italic_value == "yes") {
                flags |= italic;
            }
        }
        
        // Parse underline
        pugi::xml_attribute underline_attr = format_node.attribute("underline");
        if (underline_attr) {
            std::string underline_value = absl::AsciiStrToLower(underline_attr.value());
            if (underline_value == "true" || underline_value == "1" || underline_value == "yes") {
                flags |= underline;
            }
        }
        
        // Parse strikethrough
        pugi::xml_attribute strike_attr = format_node.attribute("strikethrough");
        if (strike_attr) {
            std::string strike_value = absl::AsciiStrToLower(strike_attr.value());
            if (strike_value == "true" || strike_value == "1" || strike_value == "yes") {
                flags |= strikethrough;
            }
        }
        
        // Parse small caps
        pugi::xml_attribute smallcaps_attr = format_node.attribute("smallCaps");
        if (smallcaps_attr) {
            std::string smallcaps_value = absl::AsciiStrToLower(smallcaps_attr.value());
            if (smallcaps_value == "true" || smallcaps_value == "1" || smallcaps_value == "yes") {
                flags |= smallcaps;
            }
        }
        
        // Parse shadow effect
        pugi::xml_attribute shadow_attr = format_node.attribute("shadow");
        if (shadow_attr) {
            std::string shadow_value = absl::AsciiStrToLower(shadow_attr.value());
            if (shadow_value == "true" || shadow_value == "1" || shadow_value == "yes") {
                flags |= shadow;
            }
        }
        
        // Parse subscript
        pugi::xml_attribute subscript_attr = format_node.attribute("subscript");
        if (subscript_attr) {
            std::string subscript_value = absl::AsciiStrToLower(subscript_attr.value());
            if (subscript_value == "true" || subscript_value == "1" || subscript_value == "yes") {
                flags |= subscript;
            }
        }
        
        // Parse superscript
        pugi::xml_attribute superscript_attr = format_node.attribute("superscript");
        if (superscript_attr) {
            std::string superscript_value = absl::AsciiStrToLower(superscript_attr.value());
            if (superscript_value == "true" || superscript_value == "1" || superscript_value == "yes") {
                flags |= superscript;
            }
        }
        
        return Result<formatting_flag>{flags};
    }
    
} // namespace duckx