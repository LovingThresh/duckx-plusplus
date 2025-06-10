/*
 * @file: HyperlinkManager.hpp
 * @brief:
 * 
 * @author: liuy
 * @date: 2025.06.09
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once

#include <string>
#include <pugixml.hpp>

namespace duckx
{
    class Document;

    class HyperlinkManager
    {
    public:
        HyperlinkManager(Document* doc, pugi::xml_document* rels_xml);

        std::string add_relationship(const std::string& url) const;

    private:
        Document* m_doc = nullptr;
        pugi::xml_document* m_rels_xml = nullptr;
    };
}