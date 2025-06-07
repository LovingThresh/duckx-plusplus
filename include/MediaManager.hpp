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

// Forward declarations to keep headers clean
namespace pugi
{
    class xml_document;
    class xml_node;
} // namespace pugi

namespace duckx
{
    class DocxFile;
    class Image;
    class Paragraph;
    class Run;

    class MediaManager
    {
    public:
        MediaManager(DocxFile* file,
            pugi::xml_document* rels_xml,
            const pugi::xml_document* doc_xml,
            pugi::xml_document* content_types_xml);

        Run add_image(const Paragraph& p, const Image& image);

    private:
        std::string add_media_to_zip(const std::string& file_path);
        std::string add_image_relationship(const std::string& media_target);
        std::string get_unique_rid();
        unsigned int get_unique_docpr_id();

        DocxFile* m_file;
        pugi::xml_document* m_rels_xml;
        pugi::xml_document* m_content_types_xml;

        int m_media_id_counter = 1;
        int m_rid_counter = 7;
        unsigned int m_docpr_id_counter = 1;
    };
} // namespace duckx
