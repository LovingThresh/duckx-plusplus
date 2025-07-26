/*!
 * @file HyperlinkManager.cpp
 * @brief Implementation of hyperlink management system
 * 
 * Handles creation and management of hyperlinks including
 * external URL relationships and XML generation.
 */
#include "HyperlinkManager.hpp"
#include "Document.hpp" // Include full header for get_unique_rid()

namespace duckx
{
    HyperlinkManager::HyperlinkManager(Document* doc, pugi::xml_document* rels_xml)
        : m_doc(doc), m_rels_xml(rels_xml)
    {}

    std::string HyperlinkManager::add_relationship(const std::string& url) const
    {
        pugi::xml_node relationships = m_rels_xml->child("Relationships");
        if (!relationships)
        {
            relationships = m_rels_xml->append_child("Relationships");
            relationships.append_attribute("xmlns") = "http://schemas.openxmlformats.org/package/2006/relationships";
        }

        // Get the unique rId from the Document object
        std::string rId = m_doc->get_next_relationship_id();

        pugi::xml_node rel_node = relationships.append_child("Relationship");
        rel_node.append_attribute("Id").set_value(rId.c_str());
        rel_node.append_attribute("Type").set_value(
                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink");
        rel_node.append_attribute("Target").set_value(url.c_str());
        rel_node.append_attribute("TargetMode").set_value("External");

        return rId;
    }
}
