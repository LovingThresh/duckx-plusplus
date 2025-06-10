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
#include "DocxFile.hpp"
#include "HeaderFooterManager.hpp"
#include "HyperlinkManager.hpp"
#include "MediaManager.hpp"
#include "HeaderFooterBase.hpp"

namespace duckx
{
    class DocxFile;
    class MediaManager;
    class HeaderFooterManager;
    class HyperlinkManager;
    class Header;
    class Footer;

    class Document
    {
    public:
        static Document open(const std::string& path);
        static Document create(const std::string& path);

        Document::Document(Document&& other) noexcept = default;
        Document& Document::operator=(Document&& other) noexcept = default;
        Document::~Document() = default;

        void save() const;
        Body& body();
        MediaManager& media() const;
        HyperlinkManager& links() const;

        std::string get_next_relationship_id();
        unsigned int get_unique_rid();

        Header& get_header(HeaderFooterType type = HeaderFooterType::DEFAULT) const;
        Footer& get_footer(HeaderFooterType type = HeaderFooterType::DEFAULT) const;

    private:
        explicit Document(std::unique_ptr<DocxFile> file);
        void load();

        std::unique_ptr<DocxFile> m_file;
        pugi::xml_document m_document_xml;
        pugi::xml_document m_rels_xml;
        pugi::xml_document m_content_types_xml;

        Body m_body;
        std::unique_ptr<MediaManager> m_media_manager;
        std::unique_ptr<HeaderFooterManager> m_hf_manager;
        std::unique_ptr<HyperlinkManager> m_link_manager;
        int m_rid_counter = 1;
    };
} // namespace duckx
