/*!
 * @file HeaderFooterBase.hpp
 * @brief Base classes for document headers and footers
 * 
 * Provides common functionality for header and footer elements
 * including content management and XML node handling. Serves as
 * the foundation for header and footer implementations.
 * 
 * @date 2025.07
 */

#pragma once

#include "BaseElement.hpp"
#include "duckx_export.h"

namespace duckx
{
    /*!
     * @brief Base class for header and footer elements
     * 
     * Provides common functionality for both headers and footers,
     * including paragraph and table management within header/footer sections.
     */
    class DUCKX_API HeaderFooterBase
    {
    public:
        virtual ~HeaderFooterBase() = default;

        /*! @brief Add a paragraph to the header/footer */
        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);
        /*! @brief Add a table to the header/footer */
        Table add_table(int rows, int cols);

    protected:
        explicit HeaderFooterBase(pugi::xml_node rootNode);

        pugi::xml_node m_rootNode; //!< The <w:hdr> or <w:ftr> XML node
        Paragraph m_paragraph;
        Table m_table;
    };

    /*!
     * @brief Document header element
     * 
     * Represents a document header that can contain paragraphs and tables.
     * Headers appear at the top of document pages.
     */
    class DUCKX_API Header : public HeaderFooterBase
    {
    public:
        explicit Header(pugi::xml_node rootNode);
    };

    /*!
     * @brief Document footer element
     * 
     * Represents a document footer that can contain paragraphs and tables.
     * Footers appear at the bottom of document pages.
     */
    class DUCKX_API Footer : public HeaderFooterBase
    {
    public:
        explicit Footer(pugi::xml_node rootNode);
    };
}
