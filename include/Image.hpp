/*
 * @file: DImage.hpp
 * @brief:
 * 
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once
#include <string>
#include <pugixml.hpp>

namespace duckx {

    // Helper to convert Pixels to English Metric Units (EMUs)
    long long pixels_to_emu(int pixels);

    class Image {
    public:
        // Constructor takes the path to the image file on disk.
        explicit Image(std::string path, int max_width_px = 600);

        // Set the desired size of the image in the document (in pixels).
        void set_size(int width_px, int height_px);

        // This is the core method for encapsulation. It generates the <w:drawing> node.
        // It needs the relationship ID and a unique drawing ID from the Document.
        void generate_drawing_xml(pugi::xml_node parent_run_node,
                                  const std::string& relationship_id,
                                  unsigned int drawing_id) const;

        const std::string& get_path() const { return m_path; }

    private:
        std::string m_path;
        long long m_width_emu = 0;
        long long m_height_emu = 0;
    };

} // namespace duckx