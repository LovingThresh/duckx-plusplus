/*
 * @file: Document.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "Document.hpp"
#include "XmlStyleParser.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "StyleManager.hpp"
#include "OutlineManager.hpp"
#include "PageLayoutManager.hpp"

namespace duckx
{
    // Hack on pugixml (可以放在一个公共的 utility 文件中)
    struct xml_string_writer : pugi::xml_writer
    {
        std::string result;
        // ReSharper disable once CppParameterMayBeConst
        void write(const void* data, size_t size) override
        {
            result.append(static_cast<const char *>(data), size);
        }
    };

    // Modern Result<T> API implementations
    Result<Document> Document::open_safe(const std::string& path)
    {
        if (path.empty()) {
            return Result<Document>(errors::invalid_argument("path", "Path cannot be empty", 
                ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
        }

        try {
            auto file = std::make_unique<DocxFile>();
            if (!file->open(path)) {
                return Result<Document>(errors::file_not_found(path, 
                    ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
            }
            return Result<Document>(Document(std::move(file)));
        } catch (const std::exception&) {
            return Result<Document>(errors::file_access_denied(path, 
                ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
        }
    }

    Result<Document> Document::create_safe(const std::string& path)
    {
        if (path.empty()) {
            return Result<Document>(errors::invalid_argument("path", "Path cannot be empty", 
                ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
        }

        try {
            auto file = std::make_unique<DocxFile>();
            if (!file->create(path)) {
                return Result<Document>(errors::file_access_denied(path, 
                    ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
            }
            return Result<Document>(Document(std::move(file)));
        } catch (const std::exception&) {
            return Result<Document>(errors::file_access_denied(path, 
                ErrorContext{__FILE__, __FUNCTION__, __LINE__}));
        }
    }

    Result<void> Document::save_safe() const
    {
        try {
            save();
            return Result<void>();
        } catch (const std::exception& e) {
            ErrorContext errorContext{__FILE__, __FUNCTION__, __LINE__};
            errorContext.with_info("operation", "save");
            errorContext.with_info("error", e.what());
            return Result<void>(errors::file_access_denied("", errorContext));
        }
    }

    // Legacy exception-based API (preserved for backward compatibility)
    Document Document::open(const std::string& path)
    {
        auto file = std::make_unique<DocxFile>();
        if (!file->open(path))
        {
            throw std::runtime_error("Failed to open file: " + path);
        }
        return Document(std::move(file));
    }

    Document Document::create(const std::string& path)
    {
        auto file = std::make_unique<DocxFile>();
        if (!file->create(path))
        {
            throw std::runtime_error("Failed to create file: " + path);
        }
        return Document(std::move(file));
    }

    Document::Document(std::unique_ptr<DocxFile> file)
        : m_file(std::move(file))
    {
        load();
    }

    void Document::load()
    {
        if (!m_file)
            return;

        const std::string xml_content = m_file->read_entry("word/document.xml");
        if (!m_document_xml.load_string(xml_content.c_str()))
        {
            throw std::runtime_error("Failed to parse word/document.xml");
        }

        pugi::xml_node bodyNode = m_document_xml.child("w:document").child("w:body");
        if (!bodyNode)
        {
            // 如果 body 节点不存在，创建一个
            pugi::xml_node docNode = m_document_xml.child("w:document");
            if (!docNode)
            {
                docNode = m_document_xml.append_child("w:document");
            }
            bodyNode = docNode.append_child("w:body");
        }

        m_body = Body(bodyNode);

        if (m_file->has_entry("word/_rels/document.xml.rels"))
        {
            m_rels_xml.load_string(m_file->read_entry("word/_rels/document.xml.rels").c_str());
        }
        else
        {
            m_rels_xml.load_string(DocxFile::get_document_rels_xml().c_str());
        }

        int max_rid = 0;
        const pugi::xml_node relationships = m_rels_xml.child("Relationships");
        if (relationships)
        {
            for (pugi::xml_node rel = relationships.child("Relationship"); rel; rel = rel.next_sibling("Relationship"))
            {
                pugi::xml_attribute id_attr = rel.attribute("Id");
                if (id_attr)
                {
                    std::string id_str = id_attr.as_string();
                    if (id_str.rfind("rId", 0) == 0)
                    {
                        // Check if it starts with "rId"
                        std::string num_part = id_str.substr(3);
                        if (!num_part.empty() && std::all_of(num_part.begin(), num_part.end(), ::isdigit))
                        {
                            try
                            {
                                int current_id = std::stoi(num_part);
                                max_rid = std::max(max_rid, current_id);
                            }
                            catch (const std::out_of_range& e)
                            {
                                std::cerr << "Out of range error while parsing rId: " << e.what() << std::endl;
                            }
                        }
                    }
                }
            }
        }
        m_rid_counter = max_rid + 1;

        if (m_file->has_entry("[Content_Types].xml"))
        {
            m_content_types_xml.load_string(m_file->read_entry("[Content_Types].xml").c_str());
        }
        else
        {
            throw std::runtime_error("[Content_Types].xml is missing.");
        }

        m_media_manager =
                std::make_unique<MediaManager>(this, m_file.get(), &m_rels_xml, &m_document_xml, &m_content_types_xml);

        m_hf_manager =
                std::make_unique<HeaderFooterManager>(this, m_file.get(), &m_document_xml, &m_rels_xml,
                                                      &m_content_types_xml);

        m_link_manager = std::make_unique<HyperlinkManager>(this, &m_rels_xml);

        // Initialize style manager
        m_style_manager = std::make_unique<StyleManager>();
        
        // Initialize outline manager
        m_outline_manager = std::make_unique<OutlineManager>(this, m_style_manager.get());
        
        // Initialize page layout manager
        m_page_layout_manager = std::make_unique<PageLayoutManager>(this, &m_document_xml);
    }

    void Document::save() const
    {
        if (!m_file)
            return;

        m_hf_manager->save_all();

        xml_string_writer writer;
        m_document_xml.print(writer, "  ", pugi::format_default);

        m_file->write_entry("word/document.xml", writer.result);

        // Generate and save styles.xml if StyleManager has styles defined
        if (m_style_manager && m_style_manager->style_count() > 0) {
            auto styles_xml_result = m_style_manager->generate_styles_xml_safe();
            if (styles_xml_result.ok()) {
                m_file->write_entry("word/styles.xml", styles_xml_result.value());
            }
        }

        xml_string_writer rels_writer;
        m_rels_xml.print(rels_writer, "", pugi::format_raw);
        m_file->write_entry("word/_rels/document.xml.rels", rels_writer.result);

        xml_string_writer content_types_writer;
        m_content_types_xml.print(content_types_writer, "", pugi::format_raw);
        m_file->write_entry("[Content_Types].xml", content_types_writer.result);

        m_file->save();
    }

    Body& Document::body()
    {
        return m_body;
    }

    const Body& Document::body() const
    {
        return m_body;
    }

    MediaManager& Document::media() const
    {
        return *m_media_manager;
    }

    HyperlinkManager& Document::links() const
    {
        return *m_link_manager;
    }

    StyleManager& Document::styles() const
    {
        return *m_style_manager;
    }

    OutlineManager& Document::outline() const
    {
        if (!m_outline_manager) {
            // Try to reinitialize if it's null (defensive programming)
            const_cast<Document*>(this)->m_outline_manager = std::make_unique<OutlineManager>(
                const_cast<Document*>(this), m_style_manager.get());
        }
        
        return *m_outline_manager;
    }

    PageLayoutManager& Document::page_layout() const
    {
        if (!m_page_layout_manager) {
            throw std::runtime_error("PageLayoutManager not initialized. Call initialize_page_layout_structure_safe() first.");
        }
        return *m_page_layout_manager;
    }

    Result<PageLayoutManager*> Document::page_layout_safe() const
    {
        if (!m_page_layout_manager) {
            return Result<PageLayoutManager*>(
                errors::validation_failed("page_layout", "PageLayoutManager not initialized. Call initialize_page_layout_structure_safe() first."));
        }
        return Result<PageLayoutManager*>(m_page_layout_manager.get());
    }

    std::string Document::get_next_relationship_id()
    {
        return "rId" + std::to_string(m_rid_counter++);
    }

    unsigned int Document::get_unique_rid()
    {
        return m_rid_counter++;
    }

    Header& Document::get_header(const HeaderFooterType type) const
    {
        return m_hf_manager->get_header(type);
    }

    Footer& Document::get_footer(const HeaderFooterType type) const
    {
        return m_hf_manager->get_footer(type);
    }
    
    // ============================================================================
    // Style Set Operations Implementation
    // ============================================================================
    
    Result<void> Document::apply_style_set_safe(const std::string& set_name)
    {
        if (!m_style_manager) {
            return Result<void>(errors::validation_failed("style_manager", 
                "Style manager not initialized",
                DUCKX_ERROR_CONTEXT()));
        }
        
        return m_style_manager->apply_style_set_safe(set_name, *this);
    }
    
    Result<void> Document::load_style_definitions_safe(const std::string& xml_file)
    {
        if (!m_style_manager) {
            return Result<void>(errors::validation_failed("style_manager", 
                "Style manager not initialized",
                DUCKX_ERROR_CONTEXT()));
        }
        
        // First load styles from the XML file
        XmlStyleParser parser;
        auto styles_result = parser.load_styles_from_file_safe(xml_file);
        if (!styles_result.ok()) {
            return Result<void>(errors::validation_failed("xml_file", 
                absl::StrFormat("Failed to load style definitions from %s", xml_file),
                DUCKX_ERROR_CONTEXT())
                .caused_by(styles_result.error()));
        }
        
        // Register each loaded style with the style manager
        for (auto& style : styles_result.value()) {
            const std::string style_name = style->name();
            const StyleType style_type = style->type();
            
            // Create appropriate style in manager based on type
            Result<Style*> created_style = Result<Style*>(errors::invalid_argument("style_type", "Invalid initial value"));
            switch (style_type) {
                case StyleType::PARAGRAPH:
                    created_style = m_style_manager->create_paragraph_style_safe(style_name);
                    break;
                case StyleType::CHARACTER:
                    created_style = m_style_manager->create_character_style_safe(style_name);
                    break;
                case StyleType::TABLE:
                    created_style = m_style_manager->create_table_style_safe(style_name);
                    break;
                case StyleType::MIXED:
                    created_style = m_style_manager->create_mixed_style_safe(style_name);
                    break;
                default:
                    continue; // Skip unsupported types
            }
            
            if (!created_style.ok()) {
                // Style might already exist, try to get it
                auto existing_style = m_style_manager->get_style_safe(style_name);
                if (!existing_style.ok()) {
                    return Result<void>(errors::style_application_failed(
                        style_name,
                        absl::StrFormat("Failed to create or find style '%s'", style_name),
                        DUCKX_ERROR_CONTEXT())
                        .caused_by(created_style.error()));
                }
                created_style = existing_style;
            }
            
            // Copy properties from loaded style to created style
            Style* target_style = created_style.value();
            
            // Set base style if present
            if (style->base_style().has_value()) {
                target_style->set_base_style_safe(style->base_style().value());
            }
            
            // Copy properties based on type
            if (style_type == StyleType::PARAGRAPH || style_type == StyleType::MIXED) {
                target_style->set_paragraph_properties_safe(style->paragraph_properties());
            }
            if (style_type == StyleType::CHARACTER || style_type == StyleType::MIXED) {
                target_style->set_character_properties_safe(style->character_properties());
            }
            if (style_type == StyleType::TABLE) {
                target_style->set_table_properties_safe(style->table_properties());
            }
        }
        
        // Also load any style sets defined in the file
        auto sets_result = parser.load_style_sets_from_file_safe(xml_file);
        if (sets_result.ok()) {
            for (const auto& style_set : sets_result.value()) {
                m_style_manager->register_style_set_safe(style_set);
            }
        }
        
        return Result<void>{};
    }
    
    Result<void> Document::apply_style_mappings_safe(const std::map<std::string, std::string>& style_mappings)
    {
        if (!m_style_manager) {
            return Result<void>(errors::validation_failed("style_manager", 
                "Style manager not initialized",
                DUCKX_ERROR_CONTEXT()));
        }
        
        return m_style_manager->apply_style_mappings_safe(*this, style_mappings);
    }
    
    Result<void> Document::register_style_set_safe(const StyleSet& style_set)
    {
        if (!m_style_manager) {
            return Result<void>(errors::validation_failed("style_manager", 
                "Style manager not initialized",
                DUCKX_ERROR_CONTEXT()));
        }
        
        return m_style_manager->register_style_set_safe(style_set);
    }
    
    Result<void> Document::initialize_page_layout_structure_safe()
    {
        try {
            // Find or create the w:document root node
            pugi::xml_node root = m_document_xml.child("w:document");
            if (!root) {
                root = m_document_xml.append_child("w:document");
                if (!root) {
                    return Result<void>(errors::xml_parse_error(
                        "Failed to create document root node"));
                }
                
                // Add required namespaces
                root.append_attribute("xmlns:w").set_value("http://schemas.openxmlformats.org/wordprocessingml/2006/main");
                root.append_attribute("xmlns:r").set_value("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
            }
            
            // Find or create the w:body node
            pugi::xml_node body = root.child("w:body");
            if (!body) {
                body = root.append_child("w:body");
                if (!body) {
                    return Result<void>(errors::xml_parse_error(
                        "Failed to create document body node"));
                }
            }
            
            // Find or create the w:sectPr node (section properties)
            pugi::xml_node sect_pr = body.child("w:sectPr");
            if (!sect_pr) {
                sect_pr = body.append_child("w:sectPr");
                if (!sect_pr) {
                    return Result<void>(errors::xml_parse_error(
                        "Failed to create section properties node"));
                }
                
                // Initialize with default page layout settings
                
                // Page size (A4 portrait by default)
                pugi::xml_node pg_sz = sect_pr.append_child("w:pgSz");
                pg_sz.append_attribute("w:w").set_value("11906");    // A4 width in twips (210mm)
                pg_sz.append_attribute("w:h").set_value("16838");    // A4 height in twips (297mm)
                pg_sz.append_attribute("w:orient").set_value("portrait");
                
                // Page margins (1 inch = 1440 twips by default)
                pugi::xml_node pg_mar = sect_pr.append_child("w:pgMar");
                pg_mar.append_attribute("w:top").set_value("1440");
                pg_mar.append_attribute("w:right").set_value("1440");
                pg_mar.append_attribute("w:bottom").set_value("1440");
                pg_mar.append_attribute("w:left").set_value("1440");
                pg_mar.append_attribute("w:header").set_value("720");
                pg_mar.append_attribute("w:footer").set_value("720");
                pg_mar.append_attribute("w:gutter").set_value("0");
                
                // Paper source
                pugi::xml_node paper_src = sect_pr.append_child("w:paperSrc");
                paper_src.append_attribute("w:first").set_value("1");
                paper_src.append_attribute("w:other").set_value("1");
                
                // Section type (continuous by default)
                pugi::xml_node sect_pr_type = sect_pr.append_child("w:type");
                sect_pr_type.append_attribute("w:val").set_value("nextPage");
            }
            
            // Update the Body object to use the new structure
            m_body = Body(body);
            
            // Debug: Print XML structure to verify it was created correctly
            #ifdef DEBUG
            xml_string_writer debug_writer;
            m_document_xml.print(debug_writer, "  ", pugi::format_default);
            std::cout << "DEBUG: Created XML structure:\n" << debug_writer.result << std::endl;
            #endif
            
            // Reinitialize all managers to use the updated XML structure
            m_page_layout_manager = std::make_unique<PageLayoutManager>(this, &m_document_xml);
            m_outline_manager = std::make_unique<OutlineManager>(this, m_style_manager.get());
            
            return Result<void>();
        }
        catch (const std::exception& e) {
            return Result<void>(errors::xml_parse_error(
                absl::StrFormat("Failed to initialize page layout structure: %s", e.what())));
        }
    }
    
    Document::Document(Document&& other) noexcept
        : m_file(std::move(other.m_file)),
          m_document_xml(std::move(other.m_document_xml)),
          m_rels_xml(std::move(other.m_rels_xml)),
          m_content_types_xml(std::move(other.m_content_types_xml)),
          m_body(std::move(other.m_body)),
          m_media_manager(std::move(other.m_media_manager)),
          m_hf_manager(std::move(other.m_hf_manager)),
          m_link_manager(std::move(other.m_link_manager)),
          m_style_manager(std::move(other.m_style_manager)),
          m_outline_manager(std::move(other.m_outline_manager)),
          m_page_layout_manager(nullptr),  // Don't move, will recreate if needed
          m_rid_counter(other.m_rid_counter)
    {
        // Critical: PageLayoutManager cannot be safely moved because it stores raw pointers
        // to Document and xml_document. We need to recreate it if it existed.
        if (other.m_page_layout_manager) {
            // Clear the source to prevent double-delete issues
            other.m_page_layout_manager.reset();
            
            // Recreate with correct pointers to this Document instance
            m_page_layout_manager = std::make_unique<PageLayoutManager>(this, &m_document_xml);
        }
        
        // Note: Other managers may need similar updates if they store Document pointers
    }
    
    Document& Document::operator=(Document&& other) noexcept
    {
        if (this != &other) {
            // Move all members
            m_file = std::move(other.m_file);
            m_document_xml = std::move(other.m_document_xml);
            m_rels_xml = std::move(other.m_rels_xml);
            m_content_types_xml = std::move(other.m_content_types_xml);
            m_body = std::move(other.m_body);
            m_media_manager = std::move(other.m_media_manager);
            m_hf_manager = std::move(other.m_hf_manager);
            m_link_manager = std::move(other.m_link_manager);
            m_style_manager = std::move(other.m_style_manager);
            m_outline_manager = std::move(other.m_outline_manager);
            m_rid_counter = other.m_rid_counter;
            
            // Critical: PageLayoutManager cannot be safely moved because it stores raw pointers
            // Handle PageLayoutManager specially
            if (other.m_page_layout_manager) {
                other.m_page_layout_manager.reset();  // Clear source
                m_page_layout_manager = std::make_unique<PageLayoutManager>(this, &m_document_xml);
            } else {
                m_page_layout_manager.reset();
            }
            
            // Note: Other managers may need similar updates if they store Document pointers
        }
        return *this;
    }
    
} // namespace duckx
