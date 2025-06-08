/*
 * @file: Document.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include <memory>
#include "Body.hpp"

#include "MediaManager.hpp"

namespace duckx
{
    class DocxFile;

    class Document
    {
    public:
        static Document open(const std::string& path);
        static Document create(const std::string& path);

        Document(Document&&) noexcept;
        Document& operator=(Document&&) noexcept;
        ~Document();

        void save() const;
        Body& body();
        MediaManager& media() const;

    private:
        explicit Document(std::unique_ptr<DocxFile> file);
        void load();

        std::unique_ptr<DocxFile> m_file;
        pugi::xml_document m_document_xml;
        pugi::xml_document m_rels_xml;
        pugi::xml_document m_content_types_xml;

        Body m_body;
        std::unique_ptr<MediaManager> m_media_manager;
    };
} // namespace duckx
