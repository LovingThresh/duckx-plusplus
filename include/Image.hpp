/*!
 * @file Image.hpp
 * @brief Image element for embedding images in DOCX documents
 * 
 * Provides functionality to embed images into DOCX documents with
 * support for various image formats and positioning options.
 */

#pragma once
#include <pugixml.hpp>
#include <string>
#include "duckx_export.h"

#include "DrawingElement.hpp"

namespace duckx
{
    /*!
     * @brief Image element for embedding images in documents
     * 
     * Represents an image that can be embedded in a DOCX document.
     * Handles image file loading, sizing, and XML generation for
     * proper DOCX formatting.
     */
    class DUCKX_API Image : public DrawingElement
    {
    public:
        /*! @brief Create an image from file path with optional width limit */
        explicit Image(std::string path, int max_width_px = 600);

        /*! @brief Generate XML drawing structure for the image */
        void generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                  unsigned int drawing_id) const override;



        /*! @brief Get the file system path of this image */
        const std::string& get_path() const;

    private:
        /*! @brief Generate inline image XML structure */
        void generate_inline_xml(pugi::xml_node drawing_node, const std::string& relationship_id,
                                 unsigned int drawing_id,
                                 const std::string& cx_str, const std::string& cy_str,
                                 const std::string& docpr_id_str) const;
        /*! @brief Generate anchored image XML structure */
        void generate_anchor_xml(pugi::xml_node drawing_node, const std::string& relationship_id,
                                 unsigned int drawing_id,
                                 const std::string& cx_str, const std::string& cy_str,
                                 const std::string& docpr_id_str) const;
        /*! @brief Add common drawing content elements */
        void add_common_drawing_content(pugi::xml_node container_node, const std::string& relationship_id,
                                        unsigned int drawing_id, const std::string& cx_str, const std::string& cy_str,
                                        const std::string& docpr_id_str) const;

        std::string m_path; //!< File system path to the image file
    };

} // namespace duckx
