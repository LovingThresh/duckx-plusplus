/*
 * @file: Document.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "Document.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include "DocxFile.hpp"

namespace duckx
{
    // Hack on pugixml (可以放在一个公共的 utility 文件中)
    struct xml_string_writer : pugi::xml_writer
    {
        std::string result;
        // ReSharper disable once CppParameterMayBeConst
        void write(const void* data, size_t size) override
        {
            result.append(static_cast<const char*>(data), size);
        }
    };

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

    Document::Document(std::unique_ptr<DocxFile> file) : m_file(std::move(file))
    {
        load();
    }

    // 移动构造函数和赋值的实现
    Document::Document(Document&& other) noexcept = default;
    Document& Document::operator=(Document&& other) noexcept = default;
    Document::~Document() = default;

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
            std::make_unique<HeaderFooterManager>(this, m_file.get(), &m_document_xml, &m_rels_xml, &m_content_types_xml);
    }

    void Document::save() const
    {
        if (!m_file)
            return;

        m_hf_manager->save_all();
        
        xml_string_writer writer;
        m_document_xml.print(writer, "  ", pugi::format_default);

        m_file->write_entry("word/document.xml", writer.result);

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

    MediaManager& Document::media() const
    {
        return *m_media_manager;
    }

    std::string Document::get_next_relationship_id()
    {
        return "rId" + std::to_string(m_rid_counter++);
    }

    unsigned int Document::get_unique_docpr_id()
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
} // namespace duckx
