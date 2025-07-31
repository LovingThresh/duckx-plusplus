/*!
 * @file BaseElement_Core.cpp
 * @brief Implementation of core base element classes and utilities
 * 
 * Contains implementations for DocxElement base class and common utility functions
 * used throughout the document element hierarchy.
 */
#include "BaseElement_Core.hpp"

#include <cctype>
#include <map>
#include <unordered_map>

namespace duckx
{
    // Mapping of XML node names to element type integers
    std::unordered_map<std::string, int> node_map = {
        {"w:p", 0},   // Paragraph
        {"w:tbl", 1}, // Table
        {"w:r", 2},   // Run
        {"w:tr", 3},  // Table row
        {"w:tc", 4}   // Table cell
    };

    DocxElement::DocxElement(const pugi::xml_node parentNode, const pugi::xml_node currentNode)
        : m_parentNode(parentNode), m_currentNode(currentNode) {}

    pugi::xml_node DocxElement::get_node() const
    {
        return m_currentNode;
    }

    bool DocxElement::has_next_sibling() const
    {
        return !find_next_any_sibling().empty();
    }

    DocxElement::SiblingInfo DocxElement::peek_next_sibling() const
    {
        const pugi::xml_node next = find_next_any_sibling();
        if (!next)
        {
            return {};
        }

        const ElementType type = determine_element_type(next);
        const std::string tag_name = next.name();

        return {type, tag_name};
    }

    pugi::xml_node DocxElement::find_next_sibling(const std::string& name) const
    {
        const pugi::xml_node sibling = m_currentNode.next_sibling(name.c_str());
        return sibling;
    }

    pugi::xml_node DocxElement::find_next_any_sibling() const
    {
        if (!m_currentNode)
            return {};

        return m_currentNode.next_sibling();
    }

    DocxElement::ElementType DocxElement::map_string_to_element_type(const std::string& node_name)
    {
        const auto it = node_map.find(node_name);
        const int type = (it != node_map.end()) ? it->second : -1;
        switch (type)
        {
            case 0: return ElementType::PARAGRAPH;
            case 1: return ElementType::TABLE;
            case 2: return ElementType::RUN;
            case 3: return ElementType::TABLE_ROW;
            case 4: return ElementType::TABLE_CELL;
            default: return ElementType::UNKNOWN;
        }
    }

    DocxElement::ElementType DocxElement::determine_element_type(const pugi::xml_node node)
    {
        if (!node)
            return ElementType::UNKNOWN;

        const std::string node_name = node.name();

        if (node_name == "w:p")
            return ElementType::PARAGRAPH;
        else if (node_name == "w:tbl")
            return ElementType::TABLE;
        else if (node_name == "w:r")
            return ElementType::RUN;
        else if (node_name == "w:tr")
            return ElementType::TABLE_ROW;
        else if (node_name == "w:tc")
            return ElementType::TABLE_CELL;
        else
            return ElementType::UNKNOWN;
    }

    /*! @brief Convert points to twips (twentieths of a point) */
    long long points_to_twips(const double pts)
    {
        return static_cast<long long>(pts * 20.0);
    }

} // namespace duckx