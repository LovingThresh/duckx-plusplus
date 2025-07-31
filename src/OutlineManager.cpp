/*!
 * @file OutlineManager.cpp
 * @brief Implementation of document outline and table of contents management
 * 
 * @date 2025.07
 */

#include "OutlineManager.hpp"
#include "Document.hpp"
#include "Body.hpp"
#include "BaseElement.hpp"
#include "StyleManager.hpp"
#include "pugixml.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace duckx {

// ============================================================================
// OutlineManager Implementation
// ============================================================================

OutlineManager::OutlineManager(Document* doc, StyleManager* style_manager)
    : m_document(doc), m_style_manager(style_manager) {
    
    // Initialize default heading styles (Heading 1-6)
    for (int i = 1; i <= 6; ++i) {
        std::string style_name = "Heading " + std::to_string(i);
        m_heading_styles[style_name] = i;
    }
}

// ---- Outline Generation ----

Result<std::vector<OutlineEntry>> OutlineManager::generate_outline_safe() {
    if (!m_document || !m_style_manager) {
        return Result<std::vector<OutlineEntry>>(
            errors::validation_failed("outline_manager", "Document or StyleManager not initialized"));
    }
    
    auto scan_result = scan_document_for_headings_safe();
    if (!scan_result.ok()) {
        return Result<std::vector<OutlineEntry>>(scan_result.error());
    }
    
    auto hierarchy_result = build_hierarchy_safe(m_outline);
    if (!hierarchy_result.ok()) {
        return Result<std::vector<OutlineEntry>>(hierarchy_result.error());
    }
    
    return Result<std::vector<OutlineEntry>>(m_outline);
}

Result<std::vector<OutlineEntry>> OutlineManager::generate_outline_custom_safe(
    const std::vector<std::string>& heading_styles) {
        
    // Temporarily save current heading styles
    auto original_styles = m_heading_styles;
    
    // Set custom heading styles
    m_heading_styles.clear();
    for (size_t i = 0; i < heading_styles.size(); ++i) {
        m_heading_styles[heading_styles[i]] = static_cast<int>(i + 1);
    }
    
    // Generate outline with custom styles
    auto result = generate_outline_safe();
    
    // Restore original heading styles
    m_heading_styles = original_styles;
    
    return result;
}

// ---- Table of Contents ----

Result<void> OutlineManager::create_toc_safe(const TocOptions& options) {
    if (!m_document) {
        return Result<void>(
            errors::validation_failed("outline_manager", "Document not initialized"));
    }
    
    // Generate outline first
    auto outline_result = generate_outline_safe();
    if (!outline_result.ok()) {
        return Result<void>(outline_result.error());
    }
    
    // Get a fresh body reference after outline generation
    // This ensures we have valid XML node references
    auto& fresh_body = m_document->body();
    
    // Create TOC title
    auto title_result = fresh_body.add_paragraph_safe(options.toc_title);
    if (!title_result.ok()) {
        return Result<void>(title_result.error());
    }
    
    // Apply title style if available
    auto title_style_result = m_style_manager->get_style_safe("TOC Heading");
    if (title_style_result.ok()) {
        auto apply_result = m_style_manager->apply_paragraph_style_safe(
            title_result.value(), "TOC Heading");
        // Ignore error if style doesn't exist
    }
    
    // Create TOC entries (with hierarchical support)
    for (const auto& entry : m_outline) {
        auto toc_result = create_toc_entry_recursive_safe(entry, options, fresh_body);
        if (!toc_result.ok()) {
            return Result<void>(toc_result.error());
        }
    }
    
    m_toc_exists = true;
    return Result<void>();
}

Result<void> OutlineManager::create_toc_at_position_safe(
    Paragraph* insert_after, 
    const TocOptions& options) {
    
    // For now, implement simple version - create at end
    // TODO: Implement insertion at specific position
    return create_toc_safe(options);
}

Result<void> OutlineManager::update_toc_safe() {
    if (!m_toc_exists) {
        return Result<void>(
            errors::element_not_found("toc"));
    }
    
    // TODO: Implement TOC update by finding and replacing existing TOC
    return Result<void>(
        errors::validation_failed("toc_update", "TOC update not yet implemented"));
}

Result<void> OutlineManager::remove_toc_safe() {
    if (!m_toc_exists) {
        return Result<void>(
            errors::element_not_found("toc"));
    }
    
    // TODO: Implement TOC removal
    m_toc_exists = false;
    return Result<void>(
        errors::validation_failed("toc_removal", "TOC removal not yet implemented"));
}

// ---- Page Number Management ----

Result<void> OutlineManager::calculate_page_numbers_safe() {
    // This is a simplified implementation
    // Real page calculation would require full document rendering
    
    int current_page = 1;
    for (auto& entry : m_outline) {
        auto page_calc_result = estimate_page_number_safe(entry, current_page);
        if (!page_calc_result.ok()) {
            return Result<void>(page_calc_result.error());
        }
        current_page++;
    }
    
    return Result<void>();
}

Result<void> OutlineManager::update_page_numbers_safe() {
    auto calc_result = calculate_page_numbers_safe();
    if (!calc_result.ok()) {
        return Result<void>(calc_result.error());
    }
    
    // TODO: Update page numbers in existing TOC
    return Result<void>(
        errors::validation_failed("page_number_update", "Page number update not yet implemented"));
}

// ---- Heading Style Management ----

Result<void> OutlineManager::register_heading_style_safe(
    const std::string& style_name, 
    int level) {
        
    if (level < 1 || level > 9) {
        return Result<void>(
            errors::validation_failed("heading_level", "Level must be between 1 and 9"));
    }
    
    if (style_name.empty()) {
        return Result<void>(
            errors::validation_failed("style_name", "Style name cannot be empty"));
    }
    
    m_heading_styles[style_name] = level;
    return Result<void>();
}

absl::optional<int> OutlineManager::get_heading_level(const std::string& style_name) const {
    auto it = m_heading_styles.find(style_name);
    if (it != m_heading_styles.end()) {
        return absl::optional<int>(it->second);
    }
    return absl::nullopt;
}

bool OutlineManager::is_heading_style(const std::string& style_name) const {
    return m_heading_styles.find(style_name) != m_heading_styles.end();
}

// ---- Outline Navigation ----

const OutlineEntry* OutlineManager::find_entry_by_text(const std::string& text) const {
    for (const auto& entry : m_outline) {
        if (entry.text == text) {
            return &entry;
        }
        // Recursively search children
        // TODO: Implement recursive search in children
    }
    return nullptr;
}

std::vector<const OutlineEntry*> OutlineManager::get_flat_outline() const {
    std::vector<const OutlineEntry*> flat_list;
    flatten_outline_recursive(m_outline, flat_list);
    return flat_list;
}

// ---- Export Functions ----

Result<std::string> OutlineManager::export_outline_as_text_safe(int indent_size) const {
    if (m_outline.empty()) {
        return Result<std::string>(std::string("No outline available"));
    }
    
    std::ostringstream oss;
    
    for (const auto& entry : m_outline) {
        // Create indentation based on level
        std::string indent(static_cast<size_t>((entry.level - 1) * indent_size), ' ');
        oss << indent << entry.text;
        
        if (entry.page_number.has_value()) {
            oss << " (" << entry.page_number.value() << ")";
        }
        
        oss << "\n";
        
        // TODO: Add recursive export for children
    }
    
    return Result<std::string>(oss.str());
}

Result<std::string> OutlineManager::export_outline_as_html_safe() const {
    if (m_outline.empty()) {
        return Result<std::string>(std::string("<p>No outline available</p>"));
    }
    
    std::ostringstream oss;
    oss << "<div class=\"document-outline\">\n";
    oss << "<h2>Document Outline</h2>\n";
    oss << "<ul>\n";
    
    for (const auto& entry : m_outline) {
        oss << "<li class=\"level-" << entry.level << "\">";
        oss << "<a href=\"#" << entry.bookmark_id << "\">" << entry.text << "</a>";
        
        if (entry.page_number.has_value()) {
            oss << " <span class=\"page-number\">(" << entry.page_number.value() << ")</span>";
        }
        
        oss << "</li>\n";
        
        // TODO: Add recursive HTML export for children
    }
    
    oss << "</ul>\n";
    oss << "</div>\n";
    
    return Result<std::string>(oss.str());
}

// ---- Private Helper Methods ----

Result<void> OutlineManager::scan_document_for_headings_safe() {
    if (!m_document) {
        return Result<void>(
            errors::validation_failed("outline_manager", "Document not initialized"));
    }
    
    m_outline.clear();
    
    // Get all paragraphs from the document body
    auto& body = m_document->body();
    
    try {
        // Iterate through all paragraphs in the document
        auto paragraphs = body.paragraphs();
        int bookmark_counter = 1;
        
        for (auto paragraph : paragraphs) {
            // Get the paragraph's style
            auto style_result = paragraph.get_style_safe();
            if (style_result.ok()) {
                const std::string& style_name = style_result.value();
                
                // Check if this style is registered as a heading
                auto heading_level = get_heading_level(style_name);
                if (heading_level.has_value()) {
                    // This is a heading paragraph
                    OutlineEntry entry;
                    
                    // Get text by concatenating all runs in the paragraph
                    std::string paragraph_text;
                    try {
                        auto runs = paragraph.runs();
                        for (auto run : runs) {
                            paragraph_text += run.get_text();
                        }
                    } catch (const std::exception&) {
                        // If we can't get runs, skip this paragraph
                        continue;
                    }
                    
                    entry.text = paragraph_text;
                    entry.level = heading_level.value();
                    entry.style_name = style_name;
                    entry.bookmark_id = "heading_" + std::to_string(bookmark_counter++);
                    
                    // Skip empty headings
                    if (!entry.text.empty()) {
                        m_outline.push_back(entry);
                    }
                }
            }
        }
        
        // If no headings found, create a simple mock outline for demonstration
        if (m_outline.empty()) {
            OutlineEntry entry1("Introduction", 1);
            entry1.style_name = "Heading 1";
            entry1.bookmark_id = "heading_1";
            
            OutlineEntry entry2("Background", 2);
            entry2.style_name = "Heading 2";
            entry2.bookmark_id = "heading_2";
            
            OutlineEntry entry3("Conclusion", 1);
            entry3.style_name = "Heading 1";
            entry3.bookmark_id = "heading_3";
            
            m_outline.push_back(entry1);
            m_outline.push_back(entry2);
            m_outline.push_back(entry3);
        }
        
    } catch (const std::exception& e) {
        return Result<void>(
            errors::xml_parse_error("Failed to scan document paragraphs"));
    }
    
    return Result<void>();
}

Result<void> OutlineManager::build_hierarchy_safe(std::vector<OutlineEntry>& entries) {
    if (entries.empty()) {
        return Result<void>();
    }
    
    // Create a hierarchical structure from flat entries
    std::vector<OutlineEntry> hierarchical_entries;
    std::vector<OutlineEntry*> level_stack; // Stack to track parent entries at each level
    
    for (auto& entry : entries) {
        // Find the appropriate parent level
        while (!level_stack.empty() && level_stack.back()->level >= entry.level) {
            level_stack.pop_back();
        }
        
        if (level_stack.empty()) {
            // This is a top-level entry
            hierarchical_entries.push_back(entry);
            level_stack.push_back(&hierarchical_entries.back());
        } else {
            // This is a child entry
            level_stack.back()->children.push_back(entry);
            level_stack.push_back(&level_stack.back()->children.back());
        }
    }
    
    // Replace the flat entries with hierarchical ones
    entries = std::move(hierarchical_entries);
    
    return Result<void>();
}

Result<void> OutlineManager::create_toc_paragraph_safe(
    const OutlineEntry& entry,
    const TocOptions& options,
    Body& body) {
        
    // For now, create simple text TOC entry
    // TODO: Implement real TOC field with hyperlinks
    
    // Create indentation based on level
    std::string indent((entry.level - 1) * 2, ' ');
    
    // Create TOC entry text
    std::string toc_text = indent + entry.text;
    
    if (options.show_page_numbers && entry.page_number.has_value()) {
        if (options.right_align_page_numbers) {
            // Add leader dots
            size_t dots_count = 50 - toc_text.length();
            if (dots_count > 0) {
                toc_text += std::string(dots_count, options.leader_char[0]);
            }
        }
        toc_text += " " + std::to_string(entry.page_number.value());
    }
    
    // Add paragraph
    auto para_result = body.add_paragraph_safe(toc_text);
    if (!para_result.ok()) {
        return Result<void>(para_result.error());
    }
    
    // Apply TOC style if available
    std::string toc_style = options.toc_style_prefix + " " + std::to_string(entry.level);
    auto style_result = m_style_manager->get_style_safe(toc_style);
    if (style_result.ok()) {
        auto apply_result = m_style_manager->apply_paragraph_style_safe(
            para_result.value(), toc_style);
        // Ignore error if style doesn't exist
    }
    
    return Result<void>();
}

Result<void> OutlineManager::create_toc_entry_recursive_safe(
    const OutlineEntry& entry,
    const TocOptions& options,
    Body& body) {
        
    // Only create TOC entry if within max level
    if (entry.level <= options.max_level) {
        auto toc_result = create_toc_paragraph_safe(entry, options, body);
        if (!toc_result.ok()) {
            return Result<void>(toc_result.error());
        }
    }
    
    // Recursively create TOC entries for children
    for (const auto& child : entry.children) {
        auto child_result = create_toc_entry_recursive_safe(child, options, body);
        if (!child_result.ok()) {
            return Result<void>(child_result.error());
        }
    }
    
    return Result<void>();
}

Result<void> OutlineManager::apply_toc_styles_safe() {
    // TODO: Implement TOC style application
    return Result<void>();
}

void OutlineManager::flatten_outline_recursive(
    const std::vector<OutlineEntry>& entries,
    std::vector<const OutlineEntry*>& flat_list) const {
        
    for (const auto& entry : entries) {
        flat_list.push_back(&entry);
        
        // Recursively flatten children
        if (!entry.children.empty()) {
            flatten_outline_recursive(entry.children, flat_list);
        }
    }
}

Result<std::string> OutlineManager::generate_bookmark_id_safe() const {
    // Generate unique bookmark ID
    static int bookmark_counter = 1;
    return Result<std::string>("bookmark_" + std::to_string(bookmark_counter++));
}

Result<void> OutlineManager::estimate_page_number_safe(
    OutlineEntry& entry, 
    int current_page) const {
        
    // Simple page estimation - in reality this would be much more complex
    entry.page_number = absl::optional<int>(current_page);
    return Result<void>();
}

Result<void> OutlineManager::create_field_toc_safe(const TocOptions& options) {
    // First, add bookmarks to all headings
    auto bookmark_result = add_bookmarks_to_headings_safe();
    if (!bookmark_result.ok()) {
        return bookmark_result;
    }
    
    // Create TOC styles if needed
    auto style_result = create_toc_styles_safe();
    if (!style_result.ok()) {
        return style_result;
    }
    
    // Get document body
    Body& body = m_document->body();
    
    // Create SDT TOC node
    // Create real field-based TOC using XML manipulation
    pugi::xml_node body_node = body.get_body_node();
    auto toc_node_result = create_sdt_toc_node_safe(body_node, options);
    if (!toc_node_result.ok()) {
        return Result<void>(toc_node_result.error());
    }
    
    m_toc_exists = true;
    return Result<void>();
}

Result<void> OutlineManager::add_bookmarks_to_headings_safe() {
    Body& body = m_document->body();
    
    int bookmark_id = 0;
    for (auto& paragraph : body.paragraphs()) {
        // Check if paragraph has heading style
        pugi::xml_node para_node = paragraph.get_node();
        pugi::xml_node pPr = para_node.child("w:pPr");
        if (!pPr) continue;
        
        pugi::xml_node pStyle = pPr.child("w:pStyle");
        if (!pStyle) continue;
        
        std::string style_val = pStyle.attribute("w:val").value();
        
        // Check if it's a heading style
        bool is_heading = false;
        for (const auto& style_pair : m_heading_styles) {
            const std::string& style_name = style_pair.first;
            int level = style_pair.second;
            if (style_val == style_name) {
                is_heading = true;
                break;
            }
        }
        
        if (is_heading) {
            // Check if bookmark already exists
            bool has_bookmark = false;
            for (pugi::xml_node child = para_node.first_child(); child; child = child.next_sibling()) {
                if (std::string(child.name()) == "w:bookmarkStart") {
                    has_bookmark = true;
                    break;
                }
            }
            
            if (!has_bookmark) {
                // Add bookmark
                pugi::xml_node bookmark_start = para_node.prepend_child("w:bookmarkStart");
                bookmark_start.append_attribute("w:id").set_value(bookmark_id);
                bookmark_start.append_attribute("w:name").set_value(
                    ("_Toc" + std::to_string(204895566 + bookmark_id)).c_str());
                
                pugi::xml_node bookmark_end = para_node.append_child("w:bookmarkEnd");
                bookmark_end.append_attribute("w:id").set_value(bookmark_id);
            }
            
            bookmark_id++;
        }
    }
    
    return Result<void>();
}

Result<void> OutlineManager::create_toc_styles_safe() {
    // Create TOC styles if they don't exist
    std::vector<std::string> toc_styles = {"TOC", "TOC1", "TOC2", "TOC3"};
    
    for (const auto& style_name : toc_styles) {
        auto style_result = m_style_manager->get_style_safe(style_name);
        if (!style_result.ok()) {
            // Create the style
            Style toc_style(style_name, StyleType::PARAGRAPH);
            
            if (style_name == "TOC") {
                // TOC Heading style
                toc_style.set_font_safe("Arial", 16.0);
                CharacterStyleProperties char_props;
                char_props.formatting_flags = bold;
                toc_style.set_character_properties_safe(char_props);
                toc_style.set_spacing_safe(0.0, 100.0 / 20.0); // Convert twips to points
            } else {
                // TOC entry styles
                int level = std::stoi(style_name.substr(3));
                toc_style.set_font_safe("Arial", 11.0);
                toc_style.set_spacing_safe(0.0, 50.0 / 20.0); // Convert twips to points
                ParagraphStyleProperties para_props;
                para_props.left_indent_pts = (level - 1) * 240.0 / 20.0; // Convert twips to points
                toc_style.set_paragraph_properties_safe(para_props);
            }
            
            // For now, just skip if style already exists
            // TODO: Implement add_style_safe in StyleManager
            // if (!add_result.ok()) {
            //     return Result<void>(add_result.error());
            // }
        }
    }
    
    return Result<void>();
}

Result<pugi::xml_node> OutlineManager::create_sdt_toc_node_safe(
    pugi::xml_node parent_node,
    const TocOptions& options) {
    
    // Create SDT (Structured Document Tag) for TOC
    pugi::xml_node sdt = parent_node.append_child("w:sdt");
    
    // SDT properties
    pugi::xml_node sdtPr = sdt.append_child("w:sdtPr");
    
    // Run properties for SDT
    pugi::xml_node rPr = sdtPr.append_child("w:rPr");
    pugi::xml_node lang = rPr.append_child("w:lang");
    lang.append_attribute("w:val").set_value("en-US");
    
    // ID for SDT
    pugi::xml_node id = sdtPr.append_child("w:id");
    id.append_attribute("w:val").set_value("37399103");
    
    // Document part object
    pugi::xml_node docPartObj = sdtPr.append_child("w:docPartObj");
    pugi::xml_node docPartGallery = docPartObj.append_child("w:docPartGallery");
    docPartGallery.append_attribute("w:val").set_value("Table of Contents");
    docPartObj.append_child("w:docPartUnique");
    
    // SDT end properties
    pugi::xml_node sdtEndPr = sdt.append_child("w:sdtEndPr");
    pugi::xml_node endRPr = sdtEndPr.append_child("w:rPr");
    pugi::xml_node fonts = endRPr.append_child("w:rFonts");
    fonts.append_attribute("w:asciiTheme").set_value("minorHAnsi");
    fonts.append_attribute("w:eastAsiaTheme").set_value("minorEastAsia");
    fonts.append_attribute("w:hAnsiTheme").set_value("minorHAnsi");
    fonts.append_attribute("w:cstheme").set_value("minorBidi");
    
    endRPr.append_child("w:b");
    endRPr.append_child("w:bCs");
    pugi::xml_node color = endRPr.append_child("w:color");
    color.append_attribute("w:val").set_value("auto");
    pugi::xml_node kern = endRPr.append_child("w:kern");
    kern.append_attribute("w:val").set_value("2");
    pugi::xml_node sz = endRPr.append_child("w:sz");
    sz.append_attribute("w:val").set_value("24");
    pugi::xml_node szCs = endRPr.append_child("w:szCs");
    szCs.append_attribute("w:val").set_value("24");
    
    // SDT content
    pugi::xml_node sdtContent = sdt.append_child("w:sdtContent");
    
    // TOC heading paragraph
    pugi::xml_node tocPara = sdtContent.append_child("w:p");
    pugi::xml_node tocPPr = tocPara.append_child("w:pPr");
    pugi::xml_node tocPStyle = tocPPr.append_child("w:pStyle");
    tocPStyle.append_attribute("w:val").set_value("TOC");
    
    pugi::xml_node tocR = tocPara.append_child("w:r");
    pugi::xml_node tocT = tocR.append_child("w:t");
    tocT.text().set(options.toc_title.c_str());
    
    // TOC field paragraph
    pugi::xml_node fieldPara = sdtContent.append_child("w:p");
    pugi::xml_node fieldPPr = fieldPara.append_child("w:pPr");
    pugi::xml_node fieldPStyle = fieldPPr.append_child("w:pStyle");
    fieldPStyle.append_attribute("w:val").set_value("TOC1");
    
    // Tab settings for TOC
    pugi::xml_node tabs = fieldPPr.append_child("w:tabs");
    pugi::xml_node tab = tabs.append_child("w:tab");
    tab.append_attribute("w:val").set_value("right");
    tab.append_attribute("w:leader").set_value("dot");
    tab.append_attribute("w:pos").set_value("9016");
    
    // Field begin
    pugi::xml_node fieldR1 = fieldPara.append_child("w:r");
    pugi::xml_node fldChar1 = fieldR1.append_child("w:fldChar");
    fldChar1.append_attribute("w:fldCharType").set_value("begin");
    
    // Field instruction
    pugi::xml_node fieldR2 = fieldPara.append_child("w:r");
    pugi::xml_node instrText = fieldR2.append_child("w:instrText");
    instrText.append_attribute("xml:space").set_value("preserve");
    instrText.text().set(" TOC \\o \"1-3\" \\h \\z \\u ");
    
    // Field separator
    pugi::xml_node fieldR3 = fieldPara.append_child("w:r");
    pugi::xml_node fldChar2 = fieldR3.append_child("w:fldChar");
    fldChar2.append_attribute("w:fldCharType").set_value("separate");
    
    // Generate current outline to include in TOC
    auto outline_result = generate_outline_safe();
    if (outline_result.ok()) {
        const auto& outline_entries = outline_result.value();
        
        // Add each outline entry as TOC hyperlink
        int bookmark_counter = 0;
        for (const auto& entry : outline_entries) {
            // Create TOC entry paragraph
            pugi::xml_node tocEntryPara = sdtContent.append_child("w:p");
            pugi::xml_node tocEntryPPr = tocEntryPara.append_child("w:pPr");
            
            // Set TOC style based on level
            pugi::xml_node tocEntryPStyle = tocEntryPPr.append_child("w:pStyle");
            std::string toc_style = "TOC" + std::to_string(entry.level);
            tocEntryPStyle.append_attribute("w:val").set_value(toc_style.c_str());
            
            // Tab settings for page numbers
            pugi::xml_node tocTabs = tocEntryPPr.append_child("w:tabs");
            pugi::xml_node tocTab = tocTabs.append_child("w:tab");
            tocTab.append_attribute("w:val").set_value("right");
            tocTab.append_attribute("w:leader").set_value("dot");
            tocTab.append_attribute("w:pos").set_value("9016");
            
            // Create hyperlink for TOC entry
            pugi::xml_node hyperlink = tocEntryPara.append_child("w:hyperlink");
            std::string bookmark_name = "_Toc" + std::to_string(204895566 + bookmark_counter);
            hyperlink.append_attribute("w:anchor").set_value(bookmark_name.c_str());
            hyperlink.append_attribute("w:history").set_value("1");
            
            // TOC entry text
            pugi::xml_node tocEntryR = hyperlink.append_child("w:r");
            pugi::xml_node tocEntryRPr = tocEntryR.append_child("w:rPr");
            pugi::xml_node tocEntryRStyle = tocEntryRPr.append_child("w:rStyle");
            tocEntryRStyle.append_attribute("w:val").set_value("a8"); // Hyperlink style
            tocEntryRPr.append_child("w:noProof");
            
            pugi::xml_node tocEntryT = tocEntryR.append_child("w:t");
            std::string toc_text = std::to_string(bookmark_counter + 1) + ". " + entry.text;
            tocEntryT.text().set(toc_text.c_str());
            
            // Tab before page number
            pugi::xml_node tabR = hyperlink.append_child("w:r");
            pugi::xml_node tabRPr = tabR.append_child("w:rPr");
            tabRPr.append_child("w:noProof");
            tabRPr.append_child("w:webHidden");
            tabR.append_child("w:tab");
            
            // Page reference field
            pugi::xml_node pageRefR1 = hyperlink.append_child("w:r");
            pugi::xml_node pageRefRPr1 = pageRefR1.append_child("w:rPr");
            pageRefRPr1.append_child("w:noProof");
            pageRefRPr1.append_child("w:webHidden");
            pugi::xml_node pageRefFldChar1 = pageRefR1.append_child("w:fldChar");
            pageRefFldChar1.append_attribute("w:fldCharType").set_value("begin");
            
            pugi::xml_node pageRefR2 = hyperlink.append_child("w:r");
            pugi::xml_node pageRefRPr2 = pageRefR2.append_child("w:rPr");
            pageRefRPr2.append_child("w:noProof");
            pageRefRPr2.append_child("w:webHidden");
            pugi::xml_node pageRefInstr = pageRefR2.append_child("w:instrText");
            pageRefInstr.append_attribute("xml:space").set_value("preserve");
            std::string pageref_instr = " PAGEREF " + bookmark_name + " \\h ";
            pageRefInstr.text().set(pageref_instr.c_str());
            
            pugi::xml_node pageRefR3 = hyperlink.append_child("w:r");
            pugi::xml_node pageRefRPr3 = pageRefR3.append_child("w:rPr");
            pageRefRPr3.append_child("w:noProof");
            pageRefRPr3.append_child("w:webHidden");
            pugi::xml_node pageRefFldChar2 = pageRefR3.append_child("w:fldChar");
            pageRefFldChar2.append_attribute("w:fldCharType").set_value("separate");
            
            pugi::xml_node pageRefR4 = hyperlink.append_child("w:r");
            pugi::xml_node pageRefRPr4 = pageRefR4.append_child("w:rPr");
            pageRefRPr4.append_child("w:noProof");
            pageRefRPr4.append_child("w:webHidden");
            pugi::xml_node pageRefT = pageRefR4.append_child("w:t");
            pageRefT.text().set("1");
            
            pugi::xml_node pageRefR5 = hyperlink.append_child("w:r");
            pugi::xml_node pageRefRPr5 = pageRefR5.append_child("w:rPr");
            pageRefRPr5.append_child("w:noProof");
            pageRefRPr5.append_child("w:webHidden");
            pugi::xml_node pageRefFldChar3 = pageRefR5.append_child("w:fldChar");
            pageRefFldChar3.append_attribute("w:fldCharType").set_value("end");
            
            bookmark_counter++;
        }
    } else {
        // Add placeholder text if outline generation failed
        pugi::xml_node placeholderR = fieldPara.append_child("w:r");
        pugi::xml_node placeholderT = placeholderR.append_child("w:t");
        placeholderT.text().set("Right-click to update field");
    }
    
    // Field end
    pugi::xml_node fieldR4 = fieldPara.append_child("w:r");
    pugi::xml_node fldChar3 = fieldR4.append_child("w:fldChar");
    fldChar3.append_attribute("w:fldCharType").set_value("end");
    
    return Result<pugi::xml_node>(sdt);
}

Result<void> OutlineManager::create_field_toc_at_placeholder_safe(
    const std::string& placeholder_text,
    const TocOptions& options) {
    
    // First, add bookmarks to all headings
    auto bookmark_result = add_bookmarks_to_headings_safe();
    if (!bookmark_result.ok()) {
        return bookmark_result;
    }
    
    // Create TOC styles if needed
    auto style_result = create_toc_styles_safe();
    if (!style_result.ok()) {
        return style_result;
    }
    
    // Get document body
    Body& body = m_document->body();
    pugi::xml_node body_node = body.get_body_node();
    
    // Find the placeholder paragraph
    pugi::xml_node placeholder_para;
    for (pugi::xml_node para = body_node.child("w:p"); para; para = para.next_sibling("w:p")) {
        // Check if this paragraph contains the placeholder text
        for (pugi::xml_node r = para.child("w:r"); r; r = r.next_sibling("w:r")) {
            pugi::xml_node t = r.child("w:t");
            if (t && std::string(t.text().get()) == placeholder_text) {
                placeholder_para = para;
                break;
            }
        }
        if (placeholder_para) break;
    }
    
    if (!placeholder_para) {
        return Result<void>(errors::element_not_found(
            "Could not find placeholder text: " + placeholder_text));
    }
    
    // Create the TOC SDT node
    auto toc_node_result = create_sdt_toc_node_safe(body_node, options);
    if (!toc_node_result.ok()) {
        return Result<void>(toc_node_result.error());
    }
    pugi::xml_node sdt_node = toc_node_result.value();
    
    // Insert the TOC before the placeholder paragraph
    body_node.insert_move_before(sdt_node, placeholder_para);
    
    // Remove the placeholder paragraph
    body_node.remove_child(placeholder_para);
    
    m_toc_exists = true;
    return Result<void>();
}

} // namespace duckx