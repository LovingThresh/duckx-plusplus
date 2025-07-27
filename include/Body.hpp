/*!
 * @file Body.hpp
 * @brief Document body content management
 * 
 * Handles the main content area of DOCX documents where paragraphs,
 * tables, and other elements are added and managed. Provides both
 * legacy and modern Result<T> APIs for content manipulation.
 * 
 * @date 2025.07
 */
#pragma once
#include "BaseElement.hpp"
#include "Error.hpp"
#include "duckx_export.h"

namespace duckx
{
    /*!
     * @brief Document body content container
     * 
     * Manages the main content area of DOCX documents. Provides access to
     * existing content and methods for adding new paragraphs and tables.
     * 
     * @example Adding content:
     * @code
     * Body& body = document.body();
     * auto para = body.add_paragraph_safe("Hello World");
     * auto table = body.add_table_safe(3, 2);
     * @endcode
     */
    class DUCKX_API Body
    {
    public:
        Body() = default;
        explicit Body(pugi::xml_node bodyNode);

        // Content access methods
        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs();
        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs() const;
        absl::enable_if_t<is_docx_element<Table>::value, ElementRange<Table>> tables();
        absl::enable_if_t<is_docx_element<Table>::value, ElementRange<Table>> tables() const;

        // Legacy API
        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none);
        Table add_table(int rows, int cols);

        // Modern Result<T> API versions
        
        /*!
         * @brief Safely adds a paragraph to the document body
         * @param text Optional text content for the paragraph
         * @param f Text formatting flags
         * @return Result containing Paragraph reference or error details
         */
        Result<Paragraph> add_paragraph_safe(const std::string& text = "", formatting_flag f = none);
        
        /*!
         * @brief Safely adds a table with specified dimensions
         * @param rows Number of table rows (must be > 0)
         * @param cols Number of table columns (must be > 0)
         * @return Result containing Table reference or error details
         */
        Result<Table> add_table_safe(int rows, int cols);

    private:
        pugi::xml_node m_bodyNode;
        Paragraph m_paragraph;
        Table m_table;
    };
} // namespace duckx
