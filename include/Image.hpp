/*
 * @file: DImage.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include <pugixml.hpp>
#include <string>

#include "DrawingElement.hpp"

namespace duckx
{
    class Image : public DrawingElement
    {
    public:
        // Constructor takes the path to the image file on disk.
        explicit Image(std::string path, int max_width_px = 600);

        // This is the core method for encapsulation. It generates the <w:drawing> node.
        // It needs the relationship ID and a unique drawing ID from the Document.
        void generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                  unsigned int drawing_id) const override;

        const std::string& get_path() const;

    private:
        std::string m_path;
    };

} // namespace duckx
