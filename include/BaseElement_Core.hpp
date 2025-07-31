/*!
 * @file BaseElement_Core.hpp
 * @brief Core base element classes and enumerations
 * 
 * Contains the foundational DocxElement abstract base class and
 * common enumerations used throughout the document element hierarchy.
 * 
 * @date 2025.07
 */
#pragma once

#include <constants.hpp>
#include <pugixml.hpp>
#include <string>
#include <utility>

#include "duckxiterator.hpp"
#include "duckx_export.h"
#include "Error.hpp"

namespace duckx
{
    /*!
     * @brief Abstract base class for all DOCX document elements
     * 
     * Provides common navigation and XML node management functionality
     * for all document elements like paragraphs, tables, runs, etc.
     */
    class DUCKX_API DocxElement
    {
    public:
        DocxElement() = default;
        DocxElement(pugi::xml_node parentNode, pugi::xml_node currentNode);
        DocxElement(const DocxElement&) = default;
        DocxElement& operator=(const DocxElement&) = default;
        DocxElement(DocxElement&&) = default;
        DocxElement& operator=(DocxElement&&) = default;

        virtual ~DocxElement() = default;
        /*! @brief Check if there's a next element of any type */
        virtual bool has_next() const = 0;
        /*! @brief Check if there's a next element of the same type */
        virtual bool has_next_same_type() const = 0;
        /*! @brief Set the parent XML node */
        virtual void set_parent(pugi::xml_node) = 0;
        /*! @brief Set the current XML node */
        virtual void set_current(pugi::xml_node) = 0;

        pugi::xml_node get_node() const;
        bool has_next_sibling() const;

        /*! @brief Enumeration of document element types */
        enum class ElementType
        {
            PARAGRAPH,   //!< Text paragraph element
            TABLE,       //!< Table element
            RUN,         //!< Text run within paragraph
            TABLE_ROW,   //!< Table row element
            TABLE_CELL,  //!< Table cell element
            UNKNOWN      //!< Unknown or unsupported element type
        };

        /*! @brief Information about sibling elements */
        struct SiblingInfo
        {
            ElementType type;     //!< Type of the sibling element
            std::string tag_name; //!< XML tag name of the sibling
            bool exists;          //!< Whether the sibling exists

            SiblingInfo()
                : type(ElementType::UNKNOWN), exists(false) {}

            SiblingInfo(const ElementType t, std::string tag)
                : type(t), tag_name(std::move(tag)), exists(true) {}
        };

        SiblingInfo peek_next_sibling() const;

    protected:
        pugi::xml_node find_next_sibling(const std::string& name) const;

        pugi::xml_node find_next_any_sibling() const;
        static ElementType map_string_to_element_type(const std::string& node_name);
        static ElementType determine_element_type(pugi::xml_node node);

        pugi::xml_node m_parentNode; // Parent node in the XML tree
        pugi::xml_node m_currentNode; // Current node in the XML tree
    };

    /*! @brief Convert points to twips (twentieths of a point) */
    long long points_to_twips(double pts);

} // namespace duckx