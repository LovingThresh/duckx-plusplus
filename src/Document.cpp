/*
 * @file: Document.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "Document.hpp"
#include "DocxFile.hpp"
#include <stdexcept>
#include <utility>

namespace duckx
{
    // Hack on pugixml (可以放在一个公共的 utility 文件中)
    struct xml_string_writer : pugi::xml_writer
    {
        std::string result;
        void write(const void* data, const size_t size) override
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

    Document::Document(std::unique_ptr<DocxFile> file)
        : m_file(std::move(file))
    {
        load();
    }

    // 移动构造函数和赋值的实现
    Document::Document(Document&& other) noexcept = default;
    Document& Document::operator=(Document&& other) noexcept = default;
    Document::~Document() = default;

    void Document::load()
    {
        if (!m_file) return;

        std::string xml_content = m_file->read_entry("word/document.xml");
        if (!m_document_xml.load_string(xml_content.c_str()))
        {
             throw std::runtime_error("Failed to parse word/document.xml");
        }

        pugi::xml_node bodyNode = m_document_xml.child("w:document").child("w:body");
        if (!bodyNode) {
            // 如果 body 节点不存在，创建一个
            pugi::xml_node docNode = m_document_xml.child("w:document");
            if (!docNode) {
                docNode = m_document_xml.append_child("w:document");
            }
            bodyNode = docNode.append_child("w:body");
        }

        m_body = Body(bodyNode);
    }

    void Document::save() const
    {
        if (!m_file) return;

        xml_string_writer writer;
        m_document_xml.print(writer, "  ", pugi::format_default);

        m_file->write_entry("word/document.xml", writer.result);

        m_file->save();
    }

    Body& Document::body()
    {
        return m_body;
    }
}