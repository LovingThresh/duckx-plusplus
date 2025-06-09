/*
 * @file: HeaderFooterManager.cpp
 * @brief:
 * 
 * @author: liuy
 * @date: 2025.06.09
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "HeaderFooterManager.hpp"
#include <stdexcept>

#include "Document.hpp"

namespace duckx
{
    struct xml_string_writer : pugi::xml_writer
    {
        std::string result;
        // ReSharper disable once CppParameterMayBeConst
        void write(const void* data, size_t size) override
        {
            result.append(static_cast<const char*>(data), size);
        }
    };

    HeaderFooterManager::HeaderFooterManager(Document* m_owner_doc, DocxFile* file, pugi::xml_document* doc_xml,
                                             pugi::xml_document* rels_xml,
                                             pugi::xml_document* content_types_xml)
        : m_doc(m_owner_doc), m_file(file), m_doc_xml(doc_xml), m_rels_xml(rels_xml),
        m_content_types_xml(content_types_xml)
    {
    }

    void HeaderFooterManager::save_all() const
    {
        for (auto const& pair: m_headers)
        {
            HeaderFooterType type = pair.first;
            if (m_hf_docs.count(type))
            {
                xml_string_writer writer;
                m_hf_docs.at(type)->print(writer, "", pugi::format_raw);
                m_file->write_entry(get_part_name_for_type(type, "header"), writer.result);
            }
        }

        for (auto const& pair: m_footers)
        {
            HeaderFooterType type = pair.first;
            if (m_hf_docs.count(type))
            {
                xml_string_writer writer;
                m_hf_docs.at(type)->print(writer, "", pugi::format_raw);
                m_file->write_entry(get_part_name_for_type(type, "footer"), writer.result);
            }
        }
    }

    std::string HeaderFooterManager::hf_type_to_string(const HeaderFooterType type)
    {
        static const std::map<HeaderFooterType, std::string> type_map = {
                {HeaderFooterType::DEFAULT, "default"},
                {HeaderFooterType::FIRST, "first"},
                {HeaderFooterType::EVEN, "even"},
                {HeaderFooterType::ODD, "odd"}
        };
        return type_map.at(type);
    }

    Header& HeaderFooterManager::get_header(const HeaderFooterType type)
    {
        if (m_headers.find(type) == m_headers.end())
        {
            pugi::xml_node root = create_hf_part("header", type);
            m_headers[type] = std::make_unique<Header>(root);
        }
        return *m_headers.at(type);
    }

    Footer& HeaderFooterManager::get_footer(const HeaderFooterType type)
    {
        if (m_footers.find(type) == m_footers.end())
        {
            pugi::xml_node root = create_hf_part("footer", type);
            m_footers[type] = std::make_unique<Footer>(root);
        }
        return *m_footers.at(type);
    }

    pugi::xml_node HeaderFooterManager::create_hf_part(const std::string& type_str, const HeaderFooterType type_enum)
    {
        // 1. Generate unique file name and relationship ID
        int& id_counter = (type_str == "header") ? m_header_id_counter : m_footer_id_counter;
        const std::string target_file = type_str + std::to_string(id_counter++) + ".xml";

        if (type_str == "header")
        {
            m_header_filenames[type_enum] = target_file;
        }
        else
        {
            m_footer_filenames[type_enum] = target_file;
        }

        const std::string rId = add_hf_relationship("word/" + target_file, type_str);

        // 2. Add content type
        const std::string part_name = "/word/" + target_file;
        const std::string content_type = "application/vnd.openxmlformats-officedocument.wordprocessingml." + type_str +
                                         "+xml";
        add_content_type(part_name, content_type);

        // 3. Create the XML content and write to a new entry in the zip
        const std::string root_tag = (type_str == "header") ? "hdr" : "ftr";
        const std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><w:)" + root_tag +
                                        " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"></w:"
                                        +
                                        root_tag + ">";
        m_file->write_entry("word/" + target_file, xml_content);

        // 4. Add reference to section properties
        add_hf_reference_to_sect_pr(rId, type_str, type_enum);

        // 5. Load the newly created XML into a pugixml document and return the root node
        auto hf_doc = std::make_unique<pugi::xml_document>();
        hf_doc->load_string(xml_content.c_str());
        const pugi::xml_node root = hf_doc->child(("w:" + root_tag).c_str());

        m_hf_docs[type_enum] = std::move(hf_doc); // Store the document to keep it in memory
        return root;
    }

    std::string HeaderFooterManager::get_part_name_for_type(const HeaderFooterType type,
                                                            const std::string& hf_keyword) const
    {
        if (hf_keyword == "header" && m_header_filenames.count(type))
        {
            return "word/" + m_header_filenames.at(type);
        }
        if (hf_keyword == "footer" && m_footer_filenames.count(type))
        {
            return "word/" + m_footer_filenames.at(type);
        }
        return "";
    }

    std::string HeaderFooterManager::add_hf_relationship(const std::string& target_file,
                                                         const std::string& hf_keyword_str) const
    {
        pugi::xml_node relationships = m_rels_xml->child("Relationships");
        std::string rId = m_doc->get_next_relationship_id();

        pugi::xml_node rel_node = relationships.append_child("Relationship");
        rel_node.append_attribute("Id").set_value(rId.c_str());
        rel_node.append_attribute("Type").set_value(
                ("http://schemas.openxmlformats.org/officeDocument/2006/relationships/" + hf_keyword_str).c_str());
        rel_node.append_attribute("Target").set_value(target_file.substr(target_file.find('/') + 1).c_str());

        return rId;
    }

    void HeaderFooterManager::add_content_type(const std::string& part_name, const std::string& content_type) const
    {
        pugi::xml_node types_node = m_content_types_xml->child("Types");
        pugi::xml_node override_node = types_node.append_child("Override");
        override_node.append_attribute("PartName").set_value(part_name.c_str());
        override_node.append_attribute("ContentType").set_value(content_type.c_str());
    }

    void HeaderFooterManager::add_hf_reference_to_sect_pr(const std::string& rId, const std::string& hf_keyword,
                                                          const HeaderFooterType type) const
    {
        pugi::xml_node sectPr = get_or_create_sect_pr();
        const std::string ref_tag = "w:" + hf_keyword + "Reference";

        pugi::xml_node ref_node = sectPr.append_child(ref_tag.c_str());

        ref_node.append_attribute("w:type").set_value(hf_type_to_string(type).c_str());
        ref_node.append_attribute("r:id").set_value(rId.c_str());

        if (type == HeaderFooterType::FIRST || type == HeaderFooterType::EVEN || type == HeaderFooterType::ODD)
        {
            if (!sectPr.child("w:titlePg"))
            {
                sectPr.prepend_child("w:titlePg");
            }
        }
    }

    pugi::xml_node HeaderFooterManager::get_or_create_sect_pr() const
    {
        pugi::xml_node body = m_doc_xml->child("w:document").child("w:body");
        if (!body)
            throw std::runtime_error("Cannot find <body> tag in document.xml");

        pugi::xml_node sectPr = body.child("w:sectPr");
        if (!sectPr)
        {
            sectPr = body.append_child("w:sectPr");
        }
        return sectPr;
    }
}
