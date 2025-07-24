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
#include "duckx_export.h"

namespace duckx
{
    class Document;

    class DUCKX_API HyperlinkManager
    {
    public:
        HyperlinkManager(Document* doc, pugi::xml_document* rels_xml);

        std::string add_relationship(const std::string& url) const;

    private:
        Document* m_doc = nullptr;
        pugi::xml_document* m_rels_xml = nullptr;
    };
}