/*!
 * @file StyleManager.cpp
 * @brief Implementation of the style management system
 * 
 * Provides comprehensive style management functionality including creation,
 * application, and built-in style libraries for DOCX documents.
 */

#include "StyleManager.hpp"
#include "BaseElement.hpp"

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
                
                if (m_paragraph_props.space_before_pts.has_value() || m_paragraph_props.space_after_pts.has_value()) {
                    xml += "    <w:spacing";
                    if (m_paragraph_props.space_before_pts.has_value()) {
                        int before_twips = static_cast<int>(m_paragraph_props.space_before_pts.value() * 20);
                        xml += absl::StrFormat(" w:before=\"%d\"", before_twips);
                    }
                    if (m_paragraph_props.space_after_pts.has_value()) {
                        int after_twips = static_cast<int>(m_paragraph_props.space_after_pts.value() * 20);
                        xml += absl::StrFormat(" w:after=\"%d\"", after_twips);
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
    
    Result<void> StyleManager::apply_style_safe(BaseElement& element, const std::string& style_name)
    {
        // This would need to determine element type and apply appropriate style
        // For now, return a placeholder implementation
        auto style_result = get_style_safe(style_name);
        if (!style_result.ok()) {
            return Result<void>(style_result.error());
        }
        
        // Element-specific application would be implemented based on element type
        return Result<void>{};
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
    
} // namespace duckx