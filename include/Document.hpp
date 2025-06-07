/*
 * @file: Document.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include "BaseElement.hpp"

struct zip_t;

namespace duckx
{
    class Document
    {
    public:
        Document();

        explicit Document(std::string);
        bool create();
        bool create(const std::string& filename);

        void file(std::string);
        void open();
        void save() const;
        bool is_open() const;
        Paragraph& paragraphs();
        Table& tables();

    private:
        static void create_basic_structure(zip_t* zip);
        static std::string get_content_types_xml();
        static std::string get_app_xml();
        static std::string get_core_xml();
        static std::string get_rels_xml();
        static std::string get_document_rels_xml();
        static std::string get_empty_document_xml();

    private:
        friend class IteratorHelper;
        std::string m_sDirectory;
        Paragraph m_paragraph;
        Table m_table;
        pugi::xml_document m_xmlDocument;
        bool m_bFlag_is_open = false;
    };
} // namespace duckx
