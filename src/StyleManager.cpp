/*!
 * @file StyleManager.cpp
 * @brief Implementation of the style management system
 * 
 * Provides comprehensive style management functionality including creation,
 * application, and built-in style libraries for DOCX documents.
 */

#include "StyleManager.hpp"
#include "BaseElement.hpp"
#include "Document.hpp"
#include "XmlStyleParser.hpp"

#include "absl/strings/str_format.h"
#include "absl/strings/ascii.h"

namespace duckx
{
    // ============================================================================
    // Style Class Implementation
    // ============================================================================
    
    Style::Style(const std::string& name, StyleType type)
        : m_name(name), m_type(type)
    {
    }
    
    Result<void> Style::set_base_style_safe(const std::string& base_style_name)
    {
        if (base_style_name.empty()) {
            return Result<void>(errors::invalid_argument("base_style_name", "Base style name cannot be empty",
                DUCKX_ERROR_CONTEXT_STYLE("set_base_style", m_name)));
        }
        
        if (base_style_name == m_name) {
            return Result<void>(errors::style_inheritance_cycle(
                absl::StrFormat("Style '%s' cannot inherit from itself", m_name),
                DUCKX_ERROR_CONTEXT_STYLE("set_base_style", m_name)));
        }
        
        m_base_style = base_style_name;
        return Result<void>{};
    }
    
    Result<void> Style::set_paragraph_properties_safe(const ParagraphStyleProperties& props)
    {
        if (m_type != StyleType::PARAGRAPH && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Cannot set paragraph properties on %s style", 
                    m_type == StyleType::CHARACTER ? "character" : "table"),
                DUCKX_ERROR_CONTEXT_STYLE("set_paragraph_properties", m_name)));
        }
        
        m_paragraph_props = props;
        return Result<void>{};
    }
    
    Result<void> Style::set_character_properties_safe(const CharacterStyleProperties& props)
    {
        if (m_type != StyleType::CHARACTER && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Cannot set character properties on %s style",
                    m_type == StyleType::PARAGRAPH ? "paragraph" : "table"),
                DUCKX_ERROR_CONTEXT_STYLE("set_character_properties", m_name)));
        }
        
        m_character_props = props;
        return Result<void>{};
    }
    
    Result<void> Style::set_table_properties_safe(const TableStyleProperties& props)
    {
        if (m_type != StyleType::TABLE && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Cannot set table properties on %s style",
                    m_type == StyleType::PARAGRAPH ? "paragraph" : "character"),
                DUCKX_ERROR_CONTEXT_STYLE("set_table_properties", m_name)));
        }
        
        m_table_props = props;
        return Result<void>{};
    }
    
    Result<void> Style::set_font_safe(const std::string& font_name, double size_pts)
    {
        if (m_type != StyleType::CHARACTER && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                "Cannot set font properties on non-character style",
                DUCKX_ERROR_CONTEXT_STYLE("set_font", m_name)));
        }
        
        if (font_name.empty()) {
            return Result<void>(errors::invalid_argument("font_name", "Font name cannot be empty",
                DUCKX_ERROR_CONTEXT_STYLE("set_font", m_name)));
        }
        
        if (size_pts <= 0 || size_pts > 1000) {
            return Result<void>(errors::invalid_font_size(
                absl::StrFormat("Font size %f is out of valid range (0-1000 pts)", size_pts),
                DUCKX_ERROR_CONTEXT_STYLE("set_font", m_name)));
        }
        
        m_character_props.font_name = font_name;
        m_character_props.font_size_pts = size_pts;
        return Result<void>{};
    }
    
    Result<void> Style::set_color_safe(const std::string& color_hex)
    {
        if (m_type != StyleType::CHARACTER && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                "Cannot set color properties on non-character style",
                DUCKX_ERROR_CONTEXT_STYLE("set_color", m_name)));
        }
        
        // Basic hex color validation (should be #RRGGBB or RRGGBB)
        std::string color = color_hex;
        if (!color.empty() && color[0] == '#') {
            color = color.substr(1);
        }
        
        if (color.length() != 6) {
            return Result<void>(errors::invalid_color_format(
                absl::StrFormat("Color '%s' is not a valid hex color (expected #RRGGBB)", color_hex),
                DUCKX_ERROR_CONTEXT_STYLE("set_color", m_name)));
        }
        
        for (char c : color) {
            if (!absl::ascii_isxdigit(c)) {
                return Result<void>(errors::invalid_color_format(
                    absl::StrFormat("Color '%s' contains invalid hex characters", color_hex),
                    DUCKX_ERROR_CONTEXT_STYLE("set_color", m_name)));
            }
        }
        
        m_character_props.font_color_hex = color;
        return Result<void>{};
    }
    
    Result<void> Style::set_alignment_safe(Alignment alignment)
    {
        if (m_type != StyleType::PARAGRAPH && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                "Cannot set alignment on non-paragraph style",
                DUCKX_ERROR_CONTEXT_STYLE("set_alignment", m_name)));
        }
        
        m_paragraph_props.alignment = alignment;
        return Result<void>{};
    }
    
    Result<void> Style::set_spacing_safe(double before_pts, double after_pts)
    {
        if (m_type != StyleType::PARAGRAPH && m_type != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                "Cannot set spacing on non-paragraph style",
                DUCKX_ERROR_CONTEXT_STYLE("set_spacing", m_name)));
        }
        
        if (before_pts < 0 || after_pts < 0) {
            return Result<void>(errors::invalid_spacing(
                absl::StrFormat("Spacing values cannot be negative (before: %f, after: %f)", 
                    before_pts, after_pts),
                DUCKX_ERROR_CONTEXT_STYLE("set_spacing", m_name)));
        }
        
        m_paragraph_props.space_before_pts = before_pts;
        m_paragraph_props.space_after_pts = after_pts;
        return Result<void>{};
    }
    
    Result<void> Style::validate_safe() const
    {
        // Basic name validation
        if (m_name.empty()) {
            return Result<void>(errors::validation_failed("style_name", "Style name cannot be empty",
                DUCKX_ERROR_CONTEXT_VALIDATION("validate_style", "name_not_empty")));
        }
        
        // Validate character properties if present
        if (m_character_props.font_size_pts.has_value()) {
            const double size = m_character_props.font_size_pts.value();
            if (size <= 0 || size > 1000) {
                return Result<void>(errors::invalid_font_size(
                    absl::StrFormat("Font size %f is out of valid range", size),
                    DUCKX_ERROR_CONTEXT_STYLE("validate", m_name)));
            }
        }
        
        return Result<void>{};
    }
    
    Result<std::string> Style::to_xml_safe() const
    {
        // This is a simplified XML generation - in a full implementation,
        // this would generate proper DOCX style XML
        // For MIXED styles, we need to decide the primary type for DOCX
        // Heading styles should be "paragraph" type in DOCX
        std::string docx_type;
        if (m_type == StyleType::PARAGRAPH || m_type == StyleType::MIXED) {
            docx_type = "paragraph";
        } else if (m_type == StyleType::CHARACTER) {
            docx_type = "character";
        } else if (m_type == StyleType::TABLE) {
            docx_type = "table";
        } else {
            docx_type = "paragraph"; // fallback
        }
        
        std::string xml = absl::StrFormat("<w:style w:type=\"%s\" w:styleId=\"%s\">\n",
            docx_type, m_name);
        
        xml += absl::StrFormat("  <w:name w:val=\"%s\"/>\n", m_name);
        
        if (m_base_style.has_value()) {
            xml += absl::StrFormat("  <w:basedOn w:val=\"%s\"/>\n", m_base_style.value());
        }
        
        // Generate paragraph properties for paragraph and mixed styles
        if (m_type == StyleType::PARAGRAPH || m_type == StyleType::MIXED) {
            bool has_para_props = m_paragraph_props.alignment.has_value() ||
                                  m_paragraph_props.space_before_pts.has_value() ||
                                  m_paragraph_props.space_after_pts.has_value() ||
                                  m_paragraph_props.line_spacing.has_value();
            
            if (has_para_props) {
                xml += "  <w:pPr>\n";
                
                if (m_paragraph_props.alignment.has_value()) {
                    std::string align_val = "left";
                    switch (m_paragraph_props.alignment.value()) {
                        case Alignment::CENTER: align_val = "center"; break;
                        case Alignment::RIGHT: align_val = "right"; break; 
                        case Alignment::BOTH: align_val = "both"; break;
                        default: break;
                    }
                    xml += absl::StrFormat("    <w:jc w:val=\"%s\"/>\n", align_val);
                }
                
                if (m_paragraph_props.space_before_pts.has_value() || 
                    m_paragraph_props.space_after_pts.has_value() || 
                    m_paragraph_props.line_spacing.has_value()) {
                    xml += "    <w:spacing";
                    if (m_paragraph_props.space_before_pts.has_value()) {
                        int before_twips = static_cast<int>(m_paragraph_props.space_before_pts.value() * 20);
                        xml += absl::StrFormat(" w:before=\"%d\"", before_twips);
                    }
                    if (m_paragraph_props.space_after_pts.has_value()) {
                        int after_twips = static_cast<int>(m_paragraph_props.space_after_pts.value() * 20);
                        xml += absl::StrFormat(" w:after=\"%d\"", after_twips);
                    }
                    if (m_paragraph_props.line_spacing.has_value()) {
                        // Convert line spacing to OOXML format (240 = single spacing)
                        int line_twips = static_cast<int>(m_paragraph_props.line_spacing.value() * 240);
                        xml += absl::StrFormat(" w:line=\"%d\" w:lineRule=\"auto\"", line_twips);
                    }
                    xml += "/>\n";
                }
                
                xml += "  </w:pPr>\n";
            }
        }
        
        // Generate character properties for character and mixed styles
        if (m_type == StyleType::CHARACTER || m_type == StyleType::MIXED) {
            bool has_char_props = m_character_props.font_name.has_value() ||
                                  m_character_props.font_size_pts.has_value() ||
                                  m_character_props.font_color_hex.has_value() ||
                                  m_character_props.formatting_flags.has_value();
            
            if (has_char_props) {
                xml += "  <w:rPr>\n";
                
                if (m_character_props.font_name.has_value()) {
                    xml += absl::StrFormat("    <w:rFonts w:ascii=\"%s\" w:hAnsi=\"%s\"/>\n", 
                        m_character_props.font_name.value(),
                        m_character_props.font_name.value());
                }
                
                if (m_character_props.font_size_pts.has_value()) {
                    int half_pts = static_cast<int>(m_character_props.font_size_pts.value() * 2);
                    xml += absl::StrFormat("    <w:sz w:val=\"%d\"/>\n", half_pts);
                    xml += absl::StrFormat("    <w:szCs w:val=\"%d\"/>\n", half_pts);
                }
                
                if (m_character_props.font_color_hex.has_value()) {
                    xml += absl::StrFormat("    <w:color w:val=\"%s\"/>\n", 
                        m_character_props.font_color_hex.value());
                }
                
                if (m_character_props.formatting_flags.has_value()) {
                    formatting_flag flags = m_character_props.formatting_flags.value();
                    if (flags & bold) {
                        xml += "    <w:b/>\n    <w:bCs/>\n";
                    }
                    if (flags & italic) {
                        xml += "    <w:i/>\n    <w:iCs/>\n";
                    }
                    if (flags & underline) {
                        xml += "    <w:u w:val=\"single\"/>\n";
                    }
                }
                
                xml += "  </w:rPr>\n";
            }
        }
        
        xml += "</w:style>\n";
        return Result<std::string>{xml};
    }
    
    // ============================================================================
    // StyleManager Class Implementation  
    // ============================================================================
    
    StyleManager::StyleManager()
    {
        // Constructor intentionally minimal - styles loaded on demand
    }
    
    Result<Style*> StyleManager::create_paragraph_style_safe(const std::string& name)
    {
        return create_style_internal_safe(name, StyleType::PARAGRAPH);
    }
    
    Result<Style*> StyleManager::create_character_style_safe(const std::string& name)
    {
        return create_style_internal_safe(name, StyleType::CHARACTER);
    }
    
    Result<Style*> StyleManager::create_table_style_safe(const std::string& name)
    {
        return create_style_internal_safe(name, StyleType::TABLE);
    }
    
    Result<Style*> StyleManager::create_mixed_style_safe(const std::string& name)
    {
        return create_style_internal_safe(name, StyleType::MIXED);
    }
    
    Result<Style*> StyleManager::get_style_safe(const std::string& name)
    {
        auto it = m_styles.find(name);
        if (it == m_styles.end()) {
            return Result<Style*>(errors::style_not_found(name, DUCKX_ERROR_CONTEXT_STYLE("get_style", name)));
        }
        return Result<Style*>{it->second.get()};
    }
    
    Result<const Style*> StyleManager::get_style_safe(const std::string& name) const
    {
        auto it = m_styles.find(name);
        if (it == m_styles.end()) {
            return Result<const Style*>(errors::style_not_found(name, DUCKX_ERROR_CONTEXT_STYLE("get_style", name)));
        }
        return Result<const Style*>{it->second.get()};
    }
    
    Result<void> StyleManager::remove_style_safe(const std::string& name)
    {
        auto it = m_styles.find(name);
        if (it == m_styles.end()) {
            return Result<void>(errors::style_not_found(name, DUCKX_ERROR_CONTEXT_STYLE("remove_style", name)));
        }
        
        // Check if this style is used as a base style by others
        for (const auto& pair : m_styles) {
            const std::string& style_name = pair.first;
            const std::unique_ptr<Style>& style = pair.second;
            if (style->base_style().has_value() && style->base_style().value() == name) {
                return Result<void>(errors::style_dependency_missing(
                    absl::StrFormat("Cannot remove style '%s' - it is used as base by '%s'", 
                        name, style_name),
                    DUCKX_ERROR_CONTEXT_STYLE("remove_style", name)));
            }
        }
        
        m_styles.erase(it);
        return Result<void>{};
    }
    
    bool StyleManager::has_style(const std::string& name) const
    {
        return m_styles.find(name) != m_styles.end();
    }
    
    Result<void> StyleManager::load_built_in_styles_safe(BuiltInStyleCategory category)
    {
        const std::string category_key = absl::StrFormat("%d", static_cast<int>(category));
        if (m_built_in_loaded_categories.count(category_key)) {
            return Result<void>{}; // Already loaded
        }
        
        Result<void> result;
        switch (category) {
            case BuiltInStyleCategory::HEADING:
                result = create_built_in_heading_styles_safe();
                break;
            case BuiltInStyleCategory::BODY_TEXT:
                result = create_built_in_body_text_styles_safe();
                break;
            case BuiltInStyleCategory::LIST:
                result = create_built_in_list_styles_safe();
                break;
            case BuiltInStyleCategory::TABLE:
                result = create_built_in_table_styles_safe();
                break;
            case BuiltInStyleCategory::TECHNICAL:
                result = create_built_in_technical_styles_safe();
                break;
            default:
                return Result<void>(errors::invalid_argument("category", "Unknown built-in style category",
                    DUCKX_ERROR_CONTEXT()));
        }
        
        if (result.ok()) {
            m_built_in_loaded_categories.insert(category_key);
        }
        return result;
    }
    
    Result<void> StyleManager::load_all_built_in_styles_safe()
    {
        const std::vector<BuiltInStyleCategory> all_categories = {
            BuiltInStyleCategory::HEADING,
            BuiltInStyleCategory::BODY_TEXT,
            BuiltInStyleCategory::LIST,
            BuiltInStyleCategory::TABLE,
            BuiltInStyleCategory::TECHNICAL
        };
        
        for (const auto category : all_categories) {
            auto result = load_built_in_styles_safe(category);
            if (!result.ok()) {
                return result;
            }
        }
        
        return Result<void>{};
    }
    
    std::vector<std::string> StyleManager::get_built_in_style_names(
        absl::optional<BuiltInStyleCategory> category) const
    {
        std::vector<std::string> names;
        
        // Return actual built-in style names that are implemented
        if (!category.has_value() || category.value() == BuiltInStyleCategory::HEADING) {
            names.insert(names.end(), {"Heading 1", "Heading 2", "Heading 3", "Heading 4", "Heading 5", "Heading 6"});
        }
        if (!category.has_value() || category.value() == BuiltInStyleCategory::BODY_TEXT) {
            names.insert(names.end(), {"Normal"});  // Only Normal is implemented
        }
        if (!category.has_value() || category.value() == BuiltInStyleCategory::LIST) {
            // No list styles implemented yet (placeholder)
        }
        if (!category.has_value() || category.value() == BuiltInStyleCategory::TABLE) {
            // No table styles implemented yet (placeholder)
        }
        if (!category.has_value() || category.value() == BuiltInStyleCategory::TECHNICAL) {
            names.insert(names.end(), {"Code"});  // Only Code is implemented
        }
        
        return names;
    }
    
    Result<void> StyleManager::apply_style_safe(DocxElement& element, const std::string& style_name)
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        // Determine element type by examining the XML node name
        pugi::xml_node element_node = element.get_node();
        if (!element_node) {
            return Result<void>(errors::xml_parse_error("Invalid element node",
                DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
        }
        
        std::string node_name = element_node.name();
        
        // Try to cast to specific types and apply appropriate style
        if (node_name == "w:p") {
            // It's a paragraph
            Paragraph* para = dynamic_cast<Paragraph*>(&element);
            if (para != nullptr) {
                return apply_paragraph_style_safe(*para, style_name);
            }
        } else if (node_name == "w:r") {
            // It's a run
            Run* run = dynamic_cast<Run*>(&element);
            if (run != nullptr) {
                return apply_character_style_safe(*run, style_name);
            }
        } else if (node_name == "w:tbl") {
            // It's a table
            Table* table = dynamic_cast<Table*>(&element);
            if (table != nullptr) {
                return apply_table_style_safe(*table, style_name);
            }
        }
        
        return Result<void>(errors::style_property_invalid("Unsupported element type for style application",
            DUCKX_ERROR_CONTEXT_STYLE("apply_style", style_name)));
    }
    
    std::vector<std::string> StyleManager::get_all_style_names() const
    {
        std::vector<std::string> names;
        names.reserve(m_styles.size());
        for (const auto& pair : m_styles) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    std::vector<std::string> StyleManager::get_style_names_by_type(StyleType type) const
    {
        std::vector<std::string> names;
        for (const auto& pair : m_styles) {
            if (pair.second->type() == type) {
                names.push_back(pair.first);
            }
        }
        return names;
    }
    
    Result<std::string> StyleManager::generate_styles_xml_safe() const
    {
        std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
        xml += "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">\n";
        
        for (const auto& pair : m_styles) {
            auto style_xml_result = pair.second->to_xml_safe();
            if (!style_xml_result.ok()) {
                return Result<std::string>(style_xml_result.error());
            }
            xml += style_xml_result.value();
        }
        
        xml += "</w:styles>\n";
        return Result<std::string>{xml};
    }
    
    Result<void> StyleManager::clear_all_styles_safe()
    {
        m_styles.clear();
        m_built_in_loaded_categories.clear();
        return Result<void>{};
    }
    
    Result<void> StyleManager::validate_all_styles_safe() const
    {
        for (const auto& pair : m_styles) {
            const std::string& name = pair.first;
            const std::unique_ptr<Style>& style = pair.second;
            auto validation_result = style->validate_safe();
            if (!validation_result.ok()) {
                return Result<void>(Error(ErrorCategory::STYLE_SYSTEM, ErrorCode::STYLE_PROPERTY_INVALID,
                    absl::StrFormat("Style '%s' validation failed", name),
                    DUCKX_ERROR_CONTEXT_STYLE("validate_all_styles", name))
                    .caused_by(validation_result.error()));
            }
        }
        return Result<void>{};
    }
    
    // ---- Private Helper Methods ----
    
    Result<Style*> StyleManager::create_style_internal_safe(const std::string& name, StyleType type)
    {
        auto name_validation = validate_style_name_safe(name);
        if (!name_validation.ok()) {
            return Result<Style*>(name_validation.error());
        }
        
        if (has_style(name)) {
            return Result<Style*>(errors::style_already_exists(name, DUCKX_ERROR_CONTEXT_STYLE("create_style", name)));
        }
        
        auto style = std::make_unique<Style>(name, type);
        Style* style_ptr = style.get();
        m_styles[name] = std::move(style);
        
        return Result<Style*>{style_ptr};
    }
    
    Result<void> StyleManager::validate_style_name_safe(const std::string& name) const
    {
        if (name.empty()) {
            return Result<void>(errors::invalid_argument("name", "Style name cannot be empty",
                DUCKX_ERROR_CONTEXT()));
        }
        
        if (name.length() > 255) {
            return Result<void>(errors::invalid_argument("name", "Style name too long (max 255 characters)",
                DUCKX_ERROR_CONTEXT()));
        }
        
        return Result<void>{};
    }
    
    Result<void> StyleManager::create_built_in_heading_styles_safe()
    {
        for (int i = 1; i <= 6; ++i) {
            const std::string name = absl::StrFormat("Heading %d", i);
            // Create as MIXED style to allow both paragraph and character properties
            auto style_result = create_style_internal_safe(name, StyleType::MIXED);
            if (!style_result.ok()) {
                return Result<void>(style_result.error());
            }
            
            Style* style = style_result.value();
            style->m_is_built_in = true;
            
            // Set heading properties
            ParagraphStyleProperties para_props;
            para_props.alignment = Alignment::LEFT;
            para_props.space_before_pts = 12.0;
            para_props.space_after_pts = 6.0;
            
            CharacterStyleProperties char_props;
            char_props.font_name = "Calibri";
            char_props.font_size_pts = 16.0 - (i - 1) * 2.0; // 16, 14, 12, 10, 8, 6
            char_props.formatting_flags = bold;
            
            auto para_result = style->set_paragraph_properties_safe(para_props);
            if (!para_result.ok()) return Result<void>(para_result.error());
            
            auto char_result = style->set_character_properties_safe(char_props);
            if (!char_result.ok()) return Result<void>(char_result.error());
        }
        
        return Result<void>{};
    }
    
    Result<void> StyleManager::create_built_in_body_text_styles_safe()
    {
        // Normal style - create as MIXED to allow both paragraph and character properties
        auto normal_result = create_style_internal_safe("Normal", StyleType::MIXED);
        if (!normal_result.ok()) return Result<void>(normal_result.error());
        
        Style* normal = normal_result.value();
        normal->m_is_built_in = true;
        
        ParagraphStyleProperties normal_para;
        normal_para.alignment = Alignment::LEFT;
        normal_para.space_after_pts = 6.0;
        
        CharacterStyleProperties normal_char;
        normal_char.font_name = "Calibri";
        normal_char.font_size_pts = 11.0;
        
        auto result = normal->set_paragraph_properties_safe(normal_para);
        if (!result.ok()) return Result<void>(result.error());
        
        result = normal->set_character_properties_safe(normal_char);
        if (!result.ok()) return Result<void>(result.error());
        
        // Additional body text styles would be created here...
        
        return Result<void>{};
    }
    
    Result<void> StyleManager::create_built_in_list_styles_safe()
    {
        // Placeholder for list style creation
        return Result<void>{};
    }
    
    Result<void> StyleManager::create_built_in_table_styles_safe()
    {
        // Placeholder for table style creation
        return Result<void>{};
    }
    
    Result<void> StyleManager::create_built_in_technical_styles_safe()
    {
        // Code style
        auto code_result = create_character_style_safe("Code");
        if (!code_result.ok()) return Result<void>(code_result.error());
        
        Style* code = code_result.value();
        code->m_is_built_in = true;
        
        CharacterStyleProperties code_char;
        code_char.font_name = "Consolas";
        code_char.font_size_pts = 10.0;
        code_char.font_color_hex = "333333";
        
        auto result = code->set_character_properties_safe(code_char);
        if (!result.ok()) return Result<void>(result.error());
        
        return Result<void>{};
    }
    
    // ============================================================================
    // Style Reading Helper Methods Implementation
    // ============================================================================
    
    Result<ParagraphStyleProperties> StyleManager::read_paragraph_properties_from_xml_safe(const pugi::xml_node& ppr_node) const
    {
        ParagraphStyleProperties props;
        
        if (!ppr_node) {
            return Result<ParagraphStyleProperties>{props};
        }
        
        // Read alignment
        pugi::xml_node jc_node = ppr_node.child("w:jc");
        if (jc_node) {
            pugi::xml_attribute val_attr = jc_node.attribute("w:val");
            if (val_attr) {
                std::string align_str = val_attr.as_string();
                if (align_str == "left") props.alignment = Alignment::LEFT;
                else if (align_str == "center") props.alignment = Alignment::CENTER;
                else if (align_str == "right") props.alignment = Alignment::RIGHT;
                else if (align_str == "both") props.alignment = Alignment::BOTH;
            }
        }
        
        // Read spacing before
        pugi::xml_node spacing_node = ppr_node.child("w:spacing");
        if (spacing_node) {
            pugi::xml_attribute before_attr = spacing_node.attribute("w:before");
            if (before_attr) {
                props.space_before_pts = before_attr.as_double() / 20.0; // Convert from twips to points
            }
            
            pugi::xml_attribute after_attr = spacing_node.attribute("w:after");
            if (after_attr) {
                props.space_after_pts = after_attr.as_double() / 20.0; // Convert from twips to points
            }
            
            pugi::xml_attribute line_attr = spacing_node.attribute("w:line");
            if (line_attr) {
                props.line_spacing = line_attr.as_double() / 240.0; // Convert from OOXML format
            }
        }
        
        // Read indentation
        pugi::xml_node ind_node = ppr_node.child("w:ind");
        if (ind_node) {
            pugi::xml_attribute left_attr = ind_node.attribute("w:left");
            if (left_attr) {
                props.left_indent_pts = left_attr.as_double() / 20.0; // Convert from twips to points
            }
            
            pugi::xml_attribute right_attr = ind_node.attribute("w:right");
            if (right_attr) {
                props.right_indent_pts = right_attr.as_double() / 20.0; // Convert from twips to points
            }
            
            pugi::xml_attribute first_line_attr = ind_node.attribute("w:firstLine");
            if (first_line_attr) {
                props.first_line_indent_pts = first_line_attr.as_double() / 20.0; // Convert from twips to points
            }
        }
        
        // Read numbering properties
        pugi::xml_node numpr_node = ppr_node.child("w:numPr");
        if (numpr_node) {
            pugi::xml_node ilvl_node = numpr_node.child("w:ilvl");
            if (ilvl_node) {
                pugi::xml_attribute val_attr = ilvl_node.attribute("w:val");
                if (val_attr) {
                    props.list_level = val_attr.as_int();
                }
            }
            
            props.list_type = ListType::BULLET; // Default assumption
        }
        
        return Result<ParagraphStyleProperties>{props};
    }
    
    Result<CharacterStyleProperties> StyleManager::read_character_properties_from_xml_safe(const pugi::xml_node& rpr_node) const
    {
        CharacterStyleProperties props;
        
        if (!rpr_node) {
            return Result<CharacterStyleProperties>{props};
        }
        
        // Read font
        pugi::xml_node rfonts_node = rpr_node.child("w:rFonts");
        if (rfonts_node) {
            pugi::xml_attribute ascii_attr = rfonts_node.attribute("w:ascii");
            if (ascii_attr) {
                props.font_name = ascii_attr.as_string();
            }
        }
        
        // Read font size
        pugi::xml_node sz_node = rpr_node.child("w:sz");
        if (sz_node) {
            pugi::xml_attribute val_attr = sz_node.attribute("w:val");
            if (val_attr) {
                props.font_size_pts = val_attr.as_double() / 2.0; // Convert from half-points to points
            }
        }
        
        // Read font color
        pugi::xml_node color_node = rpr_node.child("w:color");
        if (color_node) {
            pugi::xml_attribute val_attr = color_node.attribute("w:val");
            if (val_attr) {
                props.font_color_hex = val_attr.as_string();
            }
        }
        
        // Read highlight color
        pugi::xml_node highlight_node = rpr_node.child("w:highlight");
        if (highlight_node) {
            pugi::xml_attribute val_attr = highlight_node.attribute("w:val");
            if (val_attr) {
                std::string highlight_str = val_attr.as_string();
                // Convert string to HighlightColor enum
                if (highlight_str == "yellow") props.highlight_color = HighlightColor::YELLOW;
                else if (highlight_str == "red") props.highlight_color = HighlightColor::RED;
                else if (highlight_str == "blue") props.highlight_color = HighlightColor::BLUE;
                else if (highlight_str == "green") props.highlight_color = HighlightColor::GREEN;
                // Add more colors as needed
            }
        }
        
        // Read formatting flags
        formatting_flag flags = 0;
        if (rpr_node.child("w:b")) flags |= bold;
        if (rpr_node.child("w:i")) flags |= italic;
        if (rpr_node.child("w:u")) flags |= underline;
        if (rpr_node.child("w:strike")) flags |= strikethrough;
        if (rpr_node.child("w:smallCaps")) flags |= smallcaps;
        
        if (flags != 0) {
            props.formatting_flags = flags;
        }
        
        return Result<CharacterStyleProperties>{props};
    }
    
    Result<TableStyleProperties> StyleManager::read_table_properties_from_xml_safe(const pugi::xml_node& tblpr_node) const
    {
        TableStyleProperties props;
        
        if (!tblpr_node) {
            return Result<TableStyleProperties>{props};
        }
        
        // Read table width
        pugi::xml_node tblw_node = tblpr_node.child("w:tblW");
        if (tblw_node) {
            pugi::xml_attribute w_attr = tblw_node.attribute("w:w");
            if (w_attr) {
                props.table_width_pts = w_attr.as_double() / 20.0; // Convert from twips to points
            }
        }
        
        // Read table alignment
        pugi::xml_node jc_node = tblpr_node.child("w:jc");
        if (jc_node) {
            pugi::xml_attribute val_attr = jc_node.attribute("w:val");
            if (val_attr) {
                props.table_alignment = val_attr.as_string();
            }
        }
        
        // Read table borders
        pugi::xml_node tblborders_node = tblpr_node.child("w:tblBorders");
        if (tblborders_node) {
            pugi::xml_node top_border = tblborders_node.child("w:top");
            if (top_border) {
                pugi::xml_attribute val_attr = top_border.attribute("w:val");
                if (val_attr) {
                    props.border_style = val_attr.as_string();
                }
                
                pugi::xml_attribute sz_attr = top_border.attribute("w:sz");
                if (sz_attr) {
                    props.border_width_pts = sz_attr.as_double() / 8.0; // Convert from eighths of a point
                }
                
                pugi::xml_attribute color_attr = top_border.attribute("w:color");
                if (color_attr) {
                    props.border_color_hex = color_attr.as_string();
                }
            }
        }
        
        // Read cell margins
        pugi::xml_node tblcellmar_node = tblpr_node.child("w:tblCellMar");
        if (tblcellmar_node) {
            pugi::xml_node left_mar = tblcellmar_node.child("w:left");
            if (left_mar) {
                pugi::xml_attribute w_attr = left_mar.attribute("w:w");
                if (w_attr) {
                    props.cell_padding_pts = w_attr.as_double() / 20.0; // Convert from twips to points
                }
            }
        }
        
        return Result<TableStyleProperties>{props};
    }
    
    Result<ParagraphStyleProperties> StyleManager::resolve_paragraph_inheritance_safe(const ParagraphStyleProperties& base_props, const std::string& style_name) const
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<ParagraphStyleProperties>{base_props}; // Return base props if style not found
        }
        
        const Style* style = style_result.value();
        ParagraphStyleProperties resolved_props = base_props;
        const ParagraphStyleProperties& style_props = style->paragraph_properties();
        
        // Merge style properties with base properties (style properties take precedence)
        if (style_props.alignment.has_value()) {
            resolved_props.alignment = style_props.alignment;
        }
        if (style_props.space_before_pts.has_value()) {
            resolved_props.space_before_pts = style_props.space_before_pts;
        }
        if (style_props.space_after_pts.has_value()) {
            resolved_props.space_after_pts = style_props.space_after_pts;
        }
        if (style_props.line_spacing.has_value()) {
            resolved_props.line_spacing = style_props.line_spacing;
        }
        if (style_props.left_indent_pts.has_value()) {
            resolved_props.left_indent_pts = style_props.left_indent_pts;
        }
        if (style_props.right_indent_pts.has_value()) {
            resolved_props.right_indent_pts = style_props.right_indent_pts;
        }
        if (style_props.first_line_indent_pts.has_value()) {
            resolved_props.first_line_indent_pts = style_props.first_line_indent_pts;
        }
        if (style_props.list_type.has_value()) {
            resolved_props.list_type = style_props.list_type;
        }
        if (style_props.list_level.has_value()) {
            resolved_props.list_level = style_props.list_level;
        }
        
        // Handle inheritance from base style
        if (style->base_style().has_value()) {
            // First resolve the parent style
            auto parent_result = resolve_paragraph_inheritance_safe(base_props, style->base_style().value());
            if (parent_result.ok()) {
                ParagraphStyleProperties parent_props = parent_result.value();
                
                // Now merge current style properties on top of parent properties
                if (style_props.alignment.has_value()) {
                    parent_props.alignment = style_props.alignment;
                }
                if (style_props.space_before_pts.has_value()) {
                    parent_props.space_before_pts = style_props.space_before_pts;
                }
                if (style_props.space_after_pts.has_value()) {
                    parent_props.space_after_pts = style_props.space_after_pts;
                }
                if (style_props.line_spacing.has_value()) {
                    parent_props.line_spacing = style_props.line_spacing;
                }
                if (style_props.left_indent_pts.has_value()) {
                    parent_props.left_indent_pts = style_props.left_indent_pts;
                }
                if (style_props.right_indent_pts.has_value()) {
                    parent_props.right_indent_pts = style_props.right_indent_pts;
                }
                if (style_props.first_line_indent_pts.has_value()) {
                    parent_props.first_line_indent_pts = style_props.first_line_indent_pts;
                }
                if (style_props.list_type.has_value()) {
                    parent_props.list_type = style_props.list_type;
                }
                if (style_props.list_level.has_value()) {
                    parent_props.list_level = style_props.list_level;
                }
                
                resolved_props = parent_props;
            }
        }
        
        return Result<ParagraphStyleProperties>{resolved_props};
    }
    
    Result<CharacterStyleProperties> StyleManager::resolve_character_inheritance_safe(const CharacterStyleProperties& base_props, const std::string& style_name) const
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<CharacterStyleProperties>{base_props}; // Return base props if style not found
        }
        
        const Style* style = style_result.value();
        CharacterStyleProperties resolved_props = base_props;
        const CharacterStyleProperties& style_props = style->character_properties();
        
        // Merge style properties with base properties (style properties take precedence)
        if (style_props.font_name.has_value()) {
            resolved_props.font_name = style_props.font_name;
        }
        if (style_props.font_size_pts.has_value()) {
            resolved_props.font_size_pts = style_props.font_size_pts;
        }
        if (style_props.font_color_hex.has_value()) {
            resolved_props.font_color_hex = style_props.font_color_hex;
        }
        if (style_props.highlight_color.has_value()) {
            resolved_props.highlight_color = style_props.highlight_color;
        }
        if (style_props.formatting_flags.has_value()) {
            resolved_props.formatting_flags = style_props.formatting_flags;
        }
        
        // Handle inheritance from base style
        if (style->base_style().has_value()) {
            auto parent_result = resolve_character_inheritance_safe(resolved_props, style->base_style().value());
            if (parent_result.ok()) {
                resolved_props = parent_result.value();
            }
        }
        
        return Result<CharacterStyleProperties>{resolved_props};
    }
    
    Result<TableStyleProperties> StyleManager::resolve_table_inheritance_safe(const TableStyleProperties& base_props, const std::string& style_name) const
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<TableStyleProperties>{base_props}; // Return base props if style not found
        }
        
        const Style* style = style_result.value();
        TableStyleProperties resolved_props = base_props;
        const TableStyleProperties& style_props = style->table_properties();
        
        // Merge style properties with base properties (style properties take precedence)
        if (style_props.border_style.has_value()) {
            resolved_props.border_style = style_props.border_style;
        }
        if (style_props.border_width_pts.has_value()) {
            resolved_props.border_width_pts = style_props.border_width_pts;
        }
        if (style_props.border_color_hex.has_value()) {
            resolved_props.border_color_hex = style_props.border_color_hex;
        }
        if (style_props.cell_padding_pts.has_value()) {
            resolved_props.cell_padding_pts = style_props.cell_padding_pts;
        }
        if (style_props.table_width_pts.has_value()) {
            resolved_props.table_width_pts = style_props.table_width_pts;
        }
        if (style_props.table_alignment.has_value()) {
            resolved_props.table_alignment = style_props.table_alignment;
        }
        
        // Handle inheritance from base style
        if (style->base_style().has_value()) {
            auto parent_result = resolve_table_inheritance_safe(resolved_props, style->base_style().value());
            if (parent_result.ok()) {
                resolved_props = parent_result.value();
            }
        }
        
        return Result<TableStyleProperties>{resolved_props};
    }
    
    // ============================================================================
    // Style Reading and Extraction Implementation
    // ============================================================================
    
    Result<ParagraphStyleProperties> StyleManager::read_paragraph_properties_safe(const Paragraph& element) const
    {
        pugi::xml_node ppr = element.get_node().child("w:pPr");
        if (!ppr) {
            return Result<ParagraphStyleProperties>{ParagraphStyleProperties{}};
        }
        
        return read_paragraph_properties_from_xml_safe(ppr);
    }
    
    Result<CharacterStyleProperties> StyleManager::read_character_properties_safe(const Run& element) const
    {
        pugi::xml_node rpr = element.get_node().child("w:rPr");
        if (!rpr) {
            return Result<CharacterStyleProperties>{CharacterStyleProperties{}};
        }
        
        return read_character_properties_from_xml_safe(rpr);
    }
    
    Result<TableStyleProperties> StyleManager::read_table_properties_safe(const Table& element) const
    {
        pugi::xml_node tblpr = element.get_node().child("w:tblPr");
        if (!tblpr) {
            return Result<TableStyleProperties>{TableStyleProperties{}};
        }
        
        return read_table_properties_from_xml_safe(tblpr);
    }
    
    Result<Style*> StyleManager::extract_style_from_element_safe(const DocxElement& element, const std::string& style_name)
    {
        if (style_name.empty()) {
            return Result<Style*>(errors::validation_failed("style_name", "Style name cannot be empty",
                DUCKX_ERROR_CONTEXT_STYLE("extract_style_from_element", style_name)));
        }

        pugi::xml_node element_node = element.get_node();
        if (!element_node) {
            return Result<Style*>(errors::xml_parse_error("Invalid element node",
                DUCKX_ERROR_CONTEXT_STYLE("extract_style_from_element", style_name)));
        }
        
        StyleType style_type;
        std::string node_name = element_node.name();
        
        if (node_name == "w:p") {
            style_type = StyleType::MIXED;
        } else if (node_name == "w:r") {
            style_type = StyleType::CHARACTER;
        } else if (node_name == "w:tbl") {
            style_type = StyleType::TABLE;
        } else {
            return Result<Style*>(errors::style_property_invalid("Unsupported element type for style extraction",
                DUCKX_ERROR_CONTEXT_STYLE("extract_style_from_element", style_name)));
        }
        
        auto create_result = create_style_internal_safe(style_name, style_type);
        if (!create_result.ok()) {
            return Result<Style*>(create_result.error());
        }
        
        Style* new_style = create_result.value();
        
        if (style_type == StyleType::MIXED || style_type == StyleType::PARAGRAPH) {
            auto para_props_result = read_paragraph_properties_from_xml_safe(element_node.child("w:pPr"));
            if (para_props_result.ok()) {
                auto set_result = new_style->set_paragraph_properties_safe(para_props_result.value());
                if (!set_result.ok()) {
                    return Result<Style*>(set_result.error());
                }
            }
        }
        
        if (style_type == StyleType::MIXED || style_type == StyleType::CHARACTER) {
            auto char_props_result = read_character_properties_from_xml_safe(element_node.child("w:rPr"));
            if (char_props_result.ok()) {
                auto set_result = new_style->set_character_properties_safe(char_props_result.value());
                if (!set_result.ok()) {
                    return Result<Style*>(set_result.error());
                }
            }
        }
        
        if (style_type == StyleType::TABLE) {
            auto table_props_result = read_table_properties_from_xml_safe(element_node.child("w:tblPr"));
            if (table_props_result.ok()) {
                auto set_result = new_style->set_table_properties_safe(table_props_result.value());
                if (!set_result.ok()) {
                    return Result<Style*>(set_result.error());
                }
            }
        }
        
        return Result<Style*>{new_style};
    }
    
    Result<ParagraphStyleProperties> StyleManager::get_effective_paragraph_properties_safe(const Paragraph& paragraph) const
    {
        auto direct_props_result = read_paragraph_properties_safe(paragraph);
        if (!direct_props_result.ok()) {
            return Result<ParagraphStyleProperties>(direct_props_result.error());
        }
        
        ParagraphStyleProperties effective_props = direct_props_result.value();
        
        auto style_name_result = paragraph.get_style_safe();
        if (style_name_result.ok() && !style_name_result.value().empty()) {
            auto resolved_props_result = resolve_paragraph_inheritance_safe(effective_props, style_name_result.value());
            if (resolved_props_result.ok()) {
                effective_props = resolved_props_result.value();
            }
        }
        
        return Result<ParagraphStyleProperties>{effective_props};
    }
    
    Result<CharacterStyleProperties> StyleManager::get_effective_character_properties_safe(const Run& run) const
    {
        auto direct_props_result = read_character_properties_safe(run);
        if (!direct_props_result.ok()) {
            return Result<CharacterStyleProperties>(direct_props_result.error());
        }
        
        CharacterStyleProperties effective_props = direct_props_result.value();
        
        auto style_name_result = run.get_style_safe();
        if (style_name_result.ok() && !style_name_result.value().empty()) {
            auto resolved_props_result = resolve_character_inheritance_safe(effective_props, style_name_result.value());
            if (resolved_props_result.ok()) {
                effective_props = resolved_props_result.value();
            }
        }
        
        return Result<CharacterStyleProperties>{effective_props};
    }
    
    Result<TableStyleProperties> StyleManager::get_effective_table_properties_safe(const Table& table) const
    {
        auto direct_props_result = read_table_properties_safe(table);
        if (!direct_props_result.ok()) {
            return Result<TableStyleProperties>(direct_props_result.error());
        }
        
        TableStyleProperties effective_props = direct_props_result.value();
        
        auto style_name_result = table.get_style_safe();
        if (style_name_result.ok() && !style_name_result.value().empty()) {
            auto resolved_props_result = resolve_table_inheritance_safe(effective_props, style_name_result.value());
            if (resolved_props_result.ok()) {
                effective_props = resolved_props_result.value();
            }
        }
        
        return Result<TableStyleProperties>{effective_props};
    }
    
    Result<std::string> StyleManager::compare_styles_safe(const std::string& style1_name, const std::string& style2_name) const
    {
        auto style1_result = get_style_safe(style1_name);
        if (!style1_result.ok()) {
            return Result<std::string>(style1_result.error());
        }
        
        auto style2_result = get_style_safe(style2_name);
        if (!style2_result.ok()) {
            return Result<std::string>(style2_result.error());
        }
        
        const Style* style1 = style1_result.value();
        const Style* style2 = style2_result.value();
        
        std::string report = absl::StrFormat("Comparison between '%s' and '%s':\n\n", style1_name, style2_name);
        std::string differences;
        
        if (style1->type() != style2->type()) {
            differences += absl::StrFormat("Type difference: %d vs %d\n", static_cast<int>(style1->type()), static_cast<int>(style2->type()));
        }
        
        const auto& props1 = style1->paragraph_properties();
        const auto& props2 = style2->paragraph_properties();
        
        if (props1.alignment != props2.alignment) {
            differences += "Alignment differs\n";
        }
        if (props1.space_before_pts != props2.space_before_pts) {
            differences += "Space before differs\n";
        }
        if (props1.space_after_pts != props2.space_after_pts) {
            differences += "Space after differs\n";
        }
        
        const auto& char1 = style1->character_properties();
        const auto& char2 = style2->character_properties();
        
        if (char1.font_name != char2.font_name) {
            differences += "Font name differs\n";
        }
        if (char1.font_size_pts != char2.font_size_pts) {
            differences += "Font size differs\n";
        }
        if (char1.font_color_hex != char2.font_color_hex) {
            differences += "Font color differs\n";
        }
        
        if (differences.empty()) {
            report += "Styles are identical.\n";
        } else {
            report += differences;
        }
        
        return Result<std::string>{report};
    }
    
    // ============================================================================
    // Style Application Helper Methods Implementation
    // ============================================================================
    
    Result<void> StyleManager::apply_paragraph_properties_safe(Paragraph& paragraph, const ParagraphStyleProperties& props)
    {
        try {
            // Apply alignment
            if (props.alignment.has_value()) {
                paragraph.set_alignment(props.alignment.value());
            }
            
            // Apply spacing (before and after together)
            if (props.space_before_pts.has_value() || props.space_after_pts.has_value()) {
                double before = props.space_before_pts.has_value() ? props.space_before_pts.value() : -1;
                double after = props.space_after_pts.has_value() ? props.space_after_pts.value() : -1;
                paragraph.set_spacing(before, after);
            }
            
            // Apply line spacing
            if (props.line_spacing.has_value()) {
                paragraph.set_line_spacing(props.line_spacing.value());
            }
            
            // Apply indentation (left and right together)
            if (props.left_indent_pts.has_value() || props.right_indent_pts.has_value()) {
                double left = props.left_indent_pts.has_value() ? props.left_indent_pts.value() : -1;
                double right = props.right_indent_pts.has_value() ? props.right_indent_pts.value() : -1;
                paragraph.set_indentation(left, right);
            }
            
            // Apply first line indent
            if (props.first_line_indent_pts.has_value()) {
                paragraph.set_first_line_indent(props.first_line_indent_pts.value());
            }
            
            // Apply list properties
            if (props.list_type.has_value() && props.list_level.has_value()) {
                paragraph.set_list_style(props.list_type.value(), props.list_level.value());
            }
            
            return Result<void>{};
        }
        catch (const std::exception& e) {
            return Result<void>(errors::xml_manipulation_failed(
                absl::StrFormat("Failed to apply paragraph properties: %s", e.what()),
                DUCKX_ERROR_CONTEXT()));
        }
    }
    
    Result<void> StyleManager::apply_character_properties_safe(Run& run, const CharacterStyleProperties& props)
    {
        try {
            // Apply font name
            if (props.font_name.has_value()) {
                run.set_font(props.font_name.value());
            }
            
            // Apply font size
            if (props.font_size_pts.has_value()) {
                run.set_font_size(props.font_size_pts.value());
            }
            
            // Apply font color
            if (props.font_color_hex.has_value()) {
                run.set_color(props.font_color_hex.value());
            }
            
            // Apply highlight color
            if (props.highlight_color.has_value()) {
                run.set_highlight(props.highlight_color.value());
            }
            
            // Apply formatting flags using the existing formatting system
            if (props.formatting_flags.has_value()) {
                formatting_flag flags = props.formatting_flags.value();
                
                // The Run class doesn't have individual set_bold, set_italic methods
                // Instead, it uses the formatting_flag system through add_run or direct XML manipulation
                // For now, we'll store the flags and they can be read back through get_formatting()
                
                // Create a new run with the formatting if needed, or modify existing run
                // This is a limitation of the current Run API design
            }
            
            return Result<void>{};
        }
        catch (const std::exception& e) {
            return Result<void>(errors::xml_manipulation_failed(
                absl::StrFormat("Failed to apply character properties: %s", e.what()),
                DUCKX_ERROR_CONTEXT()));
        }
    }
    
    Result<void> StyleManager::apply_table_properties_safe(Table& table, const TableStyleProperties& props)
    {
        try {
            // Apply table width
            if (props.table_width_pts.has_value()) {
                table.set_width(props.table_width_pts.value());
            }
            
            // Apply table alignment
            if (props.table_alignment.has_value()) {
                table.set_alignment(props.table_alignment.value());
            }
            
            // Apply border style
            if (props.border_style.has_value()) {
                table.set_border_style(props.border_style.value());
            }
            
            // Apply border width
            if (props.border_width_pts.has_value()) {
                table.set_border_width(props.border_width_pts.value());
            }
            
            // Apply border color
            if (props.border_color_hex.has_value()) {
                table.set_border_color(props.border_color_hex.value());
            }
            
            // Apply cell margins (using the legacy method)
            if (props.cell_padding_pts.has_value()) {
                double padding = props.cell_padding_pts.value();
                table.set_cell_margins(padding, padding, padding, padding);
            }
            
            return Result<void>{};
        }
        catch (const std::exception& e) {
            return Result<void>(errors::xml_manipulation_failed(
                absl::StrFormat("Failed to apply table properties: %s", e.what()),
                DUCKX_ERROR_CONTEXT()));
        }
    }
    
    // ============================================================================
    // Public Style Application Methods Implementation
    // ============================================================================
    
    Result<void> StyleManager::apply_paragraph_style_safe(Paragraph& paragraph, const std::string& style_name)
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        const Style* style = style_result.value();
        
        // Check if the style is compatible with paragraphs
        if (style->type() != StyleType::PARAGRAPH && style->type() != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Cannot apply %s style to paragraph", 
                    style->type() == StyleType::CHARACTER ? "character" : "table"),
                DUCKX_ERROR_CONTEXT_STYLE("apply_paragraph_style", style_name)));
        }
        
        // Set the style reference in the paragraph XML
        pugi::xml_node para_node = paragraph.get_node();
        if (!para_node) {
            return Result<void>(errors::xml_parse_error("Invalid paragraph node",
                DUCKX_ERROR_CONTEXT_STYLE("apply_paragraph_style", style_name)));
        }
        
        pugi::xml_node ppr = para_node.child("w:pPr");
        if (!ppr) {
            ppr = para_node.prepend_child("w:pPr");
            if (!ppr) {
                return Result<void>(errors::xml_parse_error("Failed to create paragraph properties node",
                    DUCKX_ERROR_CONTEXT_STYLE("apply_paragraph_style", style_name)));
            }
        }
        
        pugi::xml_node style_ref = ppr.child("w:pStyle");
        if (!style_ref) {
            style_ref = ppr.prepend_child("w:pStyle");
            if (!style_ref) {
                return Result<void>(errors::xml_parse_error("Failed to create style reference node",
                    DUCKX_ERROR_CONTEXT_STYLE("apply_paragraph_style", style_name)));
            }
        }
        
        pugi::xml_attribute val_attr = style_ref.attribute("w:val");
        if (!val_attr) {
            val_attr = style_ref.append_attribute("w:val");
            if (!val_attr) {
                return Result<void>(errors::xml_parse_error("Failed to create style value attribute",
                    DUCKX_ERROR_CONTEXT_STYLE("apply_paragraph_style", style_name)));
            }
        }
        val_attr.set_value(style_name.c_str());
        
        // Apply the style properties
        auto apply_result = apply_paragraph_properties_safe(paragraph, style->paragraph_properties());
        if (!apply_result.ok()) {
            return Result<void>(apply_result.error());
        }
        
        return Result<void>{};
    }
    
    Result<void> StyleManager::apply_character_style_safe(Run& run, const std::string& style_name)
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        const Style* style = style_result.value();
        
        // Check if the style is compatible with runs
        if (style->type() != StyleType::CHARACTER && style->type() != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Cannot apply %s style to run",
                    style->type() == StyleType::PARAGRAPH ? "paragraph" : "table"),
                DUCKX_ERROR_CONTEXT_STYLE("apply_character_style", style_name)));
        }
        
        // Apply the style properties
        auto apply_result = apply_character_properties_safe(run, style->character_properties());
        if (!apply_result.ok()) {
            return Result<void>(apply_result.error());
        }
        
        return Result<void>{};
    }
    
    Result<void> StyleManager::apply_table_style_safe(Table& table, const std::string& style_name)
    {
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        const Style* style = style_result.value();
        
        // Check if the style is compatible with tables
        if (style->type() != StyleType::TABLE && style->type() != StyleType::MIXED) {
            return Result<void>(errors::style_property_invalid(
                absl::StrFormat("Cannot apply %s style to table",
                    style->type() == StyleType::PARAGRAPH ? "paragraph" : "character"),
                DUCKX_ERROR_CONTEXT_STYLE("apply_table_style", style_name)));
        }
        
        // Apply the style properties
        auto apply_result = apply_table_properties_safe(table, style->table_properties());
        if (!apply_result.ok()) {
            return Result<void>(apply_result.error());
        }
        
        return Result<void>{};
    }
    
    // ============================================================================
    // Style Set Management Implementation
    // ============================================================================
    
    Result<void> StyleManager::register_style_set_safe(const StyleSet& style_set)
    {
        if (style_set.name.empty()) {
            return Result<void>(errors::invalid_argument("style_set.name", "Style set name cannot be empty",
                DUCKX_ERROR_CONTEXT()));
        }
        
        // Check if style set already exists
        if (m_style_sets.find(style_set.name) != m_style_sets.end()) {
            return Result<void>(errors::style_already_exists(style_set.name,
                DUCKX_ERROR_CONTEXT_STYLE("register_style_set", style_set.name)));
        }
        
        // Validate that all referenced styles exist
        for (const auto& style_name : style_set.included_styles) {
            if (!has_style(style_name)) {
                return Result<void>(errors::style_not_found(style_name,
                    DUCKX_ERROR_CONTEXT_STYLE("register_style_set", style_set.name))
                    .caused_by(errors::validation_failed("included_styles", 
                        absl::StrFormat("Style '%s' referenced in style set does not exist", style_name))));
            }
        }
        
        // Register the style set
        m_style_sets[style_set.name] = style_set;
        return Result<void>{};
    }
    
    Result<void> StyleManager::apply_style_set_safe(const std::string& set_name, Document& doc)
    {
        auto set_it = m_style_sets.find(set_name);
        if (set_it == m_style_sets.end()) {
            return Result<void>(errors::style_not_found(set_name,
                DUCKX_ERROR_CONTEXT_STYLE("apply_style_set", set_name)));
        }
        
        const StyleSet& style_set = set_it->second;
        
        // First, ensure all styles in the set are available
        std::vector<const Style*> styles_to_apply;
        for (const auto& style_name : style_set.included_styles) {
            auto style_result = get_style_safe(style_name);
            if (!style_result.ok()) {
                return Result<void>(errors::style_application_failed(
                    style_name,
                    absl::StrFormat("Cannot apply style set '%s': style '%s' not found", 
                        set_name, style_name),
                    DUCKX_ERROR_CONTEXT()));
            }
            styles_to_apply.push_back(style_result.value());
        }
        
        // Apply styles based on their type with cascading priority:
        // 1. First apply table styles (most specific)
        // 2. Then apply paragraph styles 
        // 3. Finally apply character styles (least specific)
        // This ensures proper cascading where more specific styles override general ones
        
        // Get document body for applying styles
        auto& body = doc.body();
        
        // Phase 1: Apply table styles
        for (const auto* style : styles_to_apply) {
            if (style->type() == StyleType::TABLE || style->type() == StyleType::MIXED) {
                // Apply to all tables in the document
                auto tables = body.tables();
                for (auto& table : tables) {
                    auto apply_result = apply_table_style_safe(table, style->name());
                    if (!apply_result.ok()) {
                        // Log warning but continue with other elements
                        // In production, you might want to collect these errors
                    }
                }
            }
        }
        
        // Phase 2: Apply paragraph styles
        for (const auto* style : styles_to_apply) {
            if (style->type() == StyleType::PARAGRAPH || style->type() == StyleType::MIXED) {
                // Apply to all paragraphs in the document
                auto paragraphs = body.paragraphs();
                for (auto& paragraph : paragraphs) {
                    // Check if paragraph already has a specific style
                    // If not, apply the style from the set
                    auto current_style = paragraph.get_style_safe();
                    if (!current_style.ok() || current_style.value().empty()) {
                        auto apply_result = apply_paragraph_style_safe(paragraph, style->name());
                        if (!apply_result.ok()) {
                            // Log warning but continue
                        }
                    }
                }
            }
        }
        
        // Phase 3: Apply character styles  
        for (const auto* style : styles_to_apply) {
            if (style->type() == StyleType::CHARACTER || style->type() == StyleType::MIXED) {
                // Apply to all runs in the document
                auto paragraphs = body.paragraphs();
                for (auto& paragraph : paragraphs) {
                    auto runs = paragraph.runs();
                    for (auto& run : runs) {
                        // Check if run already has a specific style
                        auto current_style = run.get_style_safe();
                        if (!current_style.ok() || current_style.value().empty()) {
                            auto apply_result = apply_character_style_safe(run, style->name());
                            if (!apply_result.ok()) {
                                // Log warning but continue
                            }
                        }
                    }
                }
            }
        }
        
        // Style set application is complete
        return Result<void>{};
    }
    
    Result<StyleSet> StyleManager::get_style_set_safe(const std::string& set_name) const
    {
        auto set_it = m_style_sets.find(set_name);
        if (set_it == m_style_sets.end()) {
            return Result<StyleSet>(errors::style_not_found(set_name,
                DUCKX_ERROR_CONTEXT_STYLE("get_style_set", set_name)));
        }
        
        return Result<StyleSet>(set_it->second);
    }
    
    std::vector<std::string> StyleManager::list_style_sets() const
    {
        std::vector<std::string> names;
        names.reserve(m_style_sets.size());
        
        for (const auto& pair : m_style_sets) {
            names.push_back(pair.first);
        }
        
        return names;
    }
    
    Result<void> StyleManager::remove_style_set_safe(const std::string& set_name)
    {
        auto set_it = m_style_sets.find(set_name);
        if (set_it == m_style_sets.end()) {
            return Result<void>(errors::style_not_found(set_name,
                DUCKX_ERROR_CONTEXT_STYLE("remove_style_set", set_name)));
        }
        
        m_style_sets.erase(set_it);
        return Result<void>{};
    }
    
    bool StyleManager::has_style_set(const std::string& set_name) const
    {
        return m_style_sets.find(set_name) != m_style_sets.end();
    }
    
    Result<void> StyleManager::load_style_sets_from_file_safe(const std::string& filepath)
    {
        // Create XML parser instance
        XmlStyleParser parser;
        
        // Load style sets from file
        auto sets_result = parser.load_style_sets_from_file_safe(filepath);
        if (!sets_result.ok()) {
            return Result<void>(errors::validation_failed("filepath", 
                absl::StrFormat("Failed to load style sets from %s", filepath),
                DUCKX_ERROR_CONTEXT())
                .caused_by(sets_result.error()));
        }
        
        // Register each loaded style set
        for (const auto& style_set : sets_result.value()) {
            auto register_result = register_style_set_safe(style_set);
            if (!register_result.ok()) {
                // Continue loading other sets even if one fails
                // But collect the first error for reporting
                return Result<void>(errors::style_application_failed(
                    style_set.name,
                    absl::StrFormat("Failed to register style set '%s' from file", style_set.name),
                    DUCKX_ERROR_CONTEXT())
                    .caused_by(register_result.error()));
            }
        }
        
        return Result<void>{};
    }
    
    Result<void> StyleManager::apply_style_mappings_safe(Document& doc, 
        const std::map<std::string, std::string>& style_mappings)
    {
        // Apply style mappings based on element patterns
        // Common patterns:
        // - "heading1" -> apply to all first-level headings
        // - "heading*" -> apply to all headings
        // - "table" -> apply to all tables
        // - "code" -> apply to code blocks
        
        auto& body = doc.body();
        
        for (const auto& mapping : style_mappings) {
            const std::string& pattern = mapping.first;
            const std::string& style_name = mapping.second;
            // Validate style exists
            auto style_result = get_style_safe(style_name);
            if (!style_result.ok()) {
                return Result<void>(errors::style_not_found(style_name,
                    DUCKX_ERROR_CONTEXT())
                    .caused_by(errors::validation_failed("style_mappings",
                        absl::StrFormat("Style '%s' for pattern '%s' not found", style_name, pattern))));
            }
            
            const Style* style = style_result.value();
            
            // Apply based on pattern matching
            if (pattern == "heading1" || pattern == "h1") {
                // Apply to paragraphs that look like level 1 headings
                auto paragraphs = body.paragraphs();
                for (auto& para : paragraphs) {
                    // Check if this paragraph is using Heading 1 style or similar
                    auto current_style = para.get_style_safe();
                    if (current_style.ok() && 
                        (current_style.value() == "Heading 1" || 
                         current_style.value() == "heading1" ||
                         current_style.value() == "h1")) {
                        apply_paragraph_style_safe(para, style_name);
                    }
                }
            }
            else if (pattern == "heading*" || pattern == "h*") {
                // Apply to all headings
                auto paragraphs = body.paragraphs();
                for (auto& para : paragraphs) {
                    auto current_style = para.get_style_safe();
                    if (current_style.ok()) {
                        const std::string& style_str = current_style.value();
                        if (style_str.find("Heading") == 0 || 
                            style_str.find("heading") == 0 ||
                            style_str.find("h") == 0) {
                            apply_paragraph_style_safe(para, style_name);
                        }
                    }
                }
            }
            else if (pattern == "table" || pattern == "tables") {
                // Apply to all tables
                auto tables = body.tables();
                for (auto& table : tables) {
                    apply_table_style_safe(table, style_name);
                }
            }
            else if (pattern == "normal" || pattern == "body") {
                // Apply to normal body text
                auto paragraphs = body.paragraphs();
                for (auto& para : paragraphs) {
                    auto current_style = para.get_style_safe();
                    if (!current_style.ok() || 
                        current_style.value().empty() ||
                        current_style.value() == "Normal") {
                        apply_paragraph_style_safe(para, style_name);
                    }
                }
            }
            else if (pattern == "code") {
                // Apply to code blocks
                auto paragraphs = body.paragraphs();
                for (auto& para : paragraphs) {
                    auto current_style = para.get_style_safe();
                    if (current_style.ok() && 
                        (current_style.value() == "Code" || 
                         current_style.value() == "code")) {
                        apply_paragraph_style_safe(para, style_name);
                    }
                }
            }
            else {
                // Direct style name matching
                auto paragraphs = body.paragraphs();
                for (auto& para : paragraphs) {
                    auto current_style = para.get_style_safe();
                    if (current_style.ok() && current_style.value() == pattern) {
                        apply_paragraph_style_safe(para, style_name);
                    }
                }
            }
        }
        
        return Result<void>{};
    }
    
} // namespace duckx