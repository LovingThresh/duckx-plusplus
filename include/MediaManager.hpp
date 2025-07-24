/*
 * @file: MediaManager.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include <string>
#include "duckx_export.h"

// Forward declarations to keep headers clean
namespace pugi
{
    class xml_document;
    class xml_node;
} // namespace pugi

namespace duckx
{
    class Document;
    class DocxFile;
    class Image;
    class TextBox;
    class Paragraph;
    class Run;

    class DUCKX_API MediaManager
    {
    public:
        MediaManager(Document* owner_doc, DocxFile* file, pugi::xml_document* rels_xml, pugi::xml_document* doc_xml,
                     pugi::xml_document* content_types_xml);

        Run add_image(const Paragraph& p, const Image& image);
        Run add_textbox(const Paragraph& p, const TextBox& textbox);

    private:
        std::string add_media_to_zip(const std::string& file_path);
        std::string add_image_relationship(const std::string& media_target) const;
        unsigned int get_unique_docpr_id();

        DocxFile* m_file = nullptr;
        pugi::xml_document* m_rels_xml = nullptr;
        pugi::xml_document* m_doc_xml = nullptr;
        pugi::xml_document* m_content_types_xml = nullptr;

        Document* m_doc = nullptr;
        int m_media_id_counter = 1;
        unsigned int m_docpr_id_counter = 1;
    };
} // namespace duckx
