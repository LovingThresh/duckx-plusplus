/*!
 * @file TextBox.hpp
 * @brief TextBox element for creating floating text containers
 * 
 * Provides functionality to create text boxes that can contain paragraphs
 * and be positioned as drawing elements within DOCX documents. Supports
 * border styles and comprehensive paragraph management.
 * 
 * @date 2025.07
 */

#pragma once
#include "BaseElement.hpp"
#include "DrawingElement.hpp"
#include "duckxiterator.hpp"
#include "duckx_export.h"

namespace duckx
{
    /*! @brief Border styles for text boxes */
    enum class DUCKX_API BorderStyle
    {
        NONE,  //!< No border
        SOLID  //!< Solid line border
    };

    /*!
     * @brief Text box element for floating text containers
     * 
     * Represents a text box that can contain paragraphs and be positioned
     * as a drawing element within the document. Text boxes can have borders
     * and support various formatting options.
     */
    class DUCKX_API TextBox : public DrawingElement
    {
    public:
        /*! @brief Create a text box with no border */
        TextBox();
        /*! @brief Create a text box with specified border style */
        explicit TextBox(BorderStyle border);

        /*! @brief Set the border style for this text box */
        void set_border(BorderStyle border);
        /*! @brief Generate XML drawing structure for the text box */
        void generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                  unsigned int drawing_id) const override;
        /*! @brief Add a paragraph to the text box */
        Paragraph add_paragraph(const std::string& text = "", formatting_flag f = none) const;
        /*! @brief Add a new paragraph to the text box */
        void add_new_paragraph(const std::string& text = "", formatting_flag f = none) const;
        /*! @brief Get the last paragraph in the text box */
        Paragraph last_paragraph() const;
        /*! @brief Get iterator range over all paragraphs */
        absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> paragraphs() const;

    private:
        mutable pugi::xml_document m_internal_doc; //!< Internal XML document for text box content
        mutable bool m_is_empty = true;            //!< Whether the text box is empty
        BorderStyle m_border_style = BorderStyle::NONE; //!< Current border style
    };
} // namespace duckx
