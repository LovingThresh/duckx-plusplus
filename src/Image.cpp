/*
 * @file: Image.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include "Image.hpp"

#include <iostream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace duckx
{

    // 1 inch = 914400 EMUs. Common screen DPI is 96.
    long long pixels_to_emu(const int pixels)
    {
        if (pixels <= 0)
            return 0;
        constexpr int dpi = 96;
        return static_cast<long long>(pixels) * 914400 / dpi;
    }

    Image::Image(std::string path, const int max_width_px)
        : m_path(std::move(path))
    {
        if (m_path.empty())
        {
            throw std::invalid_argument("Image path cannot be empty.");
        }

        int original_width = 0;
        int original_height = 0;
        int channels = 0;

        if (stbi_info(m_path.c_str(), &original_width, &original_height, &channels))
        {
            // Successfully got original image dimensions.

            int final_width = original_width;
            int final_height = original_height;

            // Check if the image is wider than the allowed maximum width.
            if (max_width_px > 0 && original_width > max_width_px)
            {
                // It's too wide, let's scale it down proportionally.
                const double ratio = static_cast<double>(max_width_px) / original_width;
                final_width = max_width_px;
                final_height = static_cast<int>(original_height * ratio);
            }

            // Set the final calculated size.
            set_size(final_width, final_height);
        }
        else
        {
            // Fallback for failed detection.
            std::cerr << "Warning: Could not get image info for '" << m_path
                      << "'. Using default size. Reason: " << stbi_failure_reason() << std::endl;
            set_size(320, 240);
        }
    }

    void Image::set_size(const int width_px, const int height_px)
    {
        m_width_emu = pixels_to_emu(width_px);
        m_height_emu = pixels_to_emu(height_px);
    }

    // This is where the complex XML generation is now cleanly encapsulated.
    void Image::generate_drawing_xml(pugi::xml_node parent_run_node,
                                     const std::string& relationship_id,
                                     const unsigned int drawing_id) const
    {
        if (!parent_run_node)
        {
            return;
        }

        pugi::xml_node drawing_node = parent_run_node.append_child("w:drawing");

        // Convert numeric values to strings once
        const std::string docpr_id_str = std::to_string(drawing_id);
        const std::string cx_str = std::to_string(m_width_emu);
        const std::string cy_str = std::to_string(m_height_emu);

        // --- Start building the complex structure, one node at a time ---
        // The structure remains the same, but local namespace declarations are removed.

        // <wp:inline distT="0" distB="0" distL="0" distR="0">
        pugi::xml_node inline_node = drawing_node.append_child("wp:inline");
        inline_node.append_attribute("distT").set_value("0");
        inline_node.append_attribute("distB").set_value("0");
        inline_node.append_attribute("distL").set_value("0");
        inline_node.append_attribute("distR").set_value("0");

        // <wp:extent cx="..." cy="...">
        pugi::xml_node extent_node = inline_node.append_child("wp:extent");
        extent_node.append_attribute("cx").set_value(cx_str.c_str());
        extent_node.append_attribute("cy").set_value(cy_str.c_str());

        // <wp:effectExtent l="0" t="0" r="0" b="0"/>
        pugi::xml_node effect_extent_node = inline_node.append_child("wp:effectExtent");
        effect_extent_node.append_attribute("l").set_value("0");
        effect_extent_node.append_attribute("t").set_value("0");
        effect_extent_node.append_attribute("r").set_value("0");
        effect_extent_node.append_attribute("b").set_value("0");

        // <wp:docPr id="..." name="..."/>
        pugi::xml_node docPr_node = inline_node.append_child("wp:docPr");
        docPr_node.append_attribute("id").set_value(docpr_id_str.c_str());
        docPr_node.append_attribute("name").set_value(("Picture " + docpr_id_str).c_str());

        // <wp:cNvGraphicFramePr>
        pugi::xml_node cNvGraphicFramePr_node = inline_node.append_child("wp:cNvGraphicFramePr");
        pugi::xml_node graphicFrameLocks_node = cNvGraphicFramePr_node.append_child("a:graphicFrameLocks");
        // REMOVED: graphicFrameLocks_node.append_attribute("xmlns:a")...
        graphicFrameLocks_node.append_attribute("noChangeAspect").set_value("1");

        // <a:graphic>
        pugi::xml_node graphic_node = inline_node.append_child("a:graphic");
        // REMOVED: graphic_node.append_attribute("xmlns:a")...

        // <a:graphicData uri="...">
        pugi::xml_node graphicData_node = graphic_node.append_child("a:graphicData");
        graphicData_node.append_attribute("uri").set_value("http://schemas.openxmlformats.org/drawingml/2006/picture");

        // <pic:pic>
        pugi::xml_node pic_node = graphicData_node.append_child("pic:pic");
        // REMOVED: pic_node.append_attribute("xmlns:pic")...

        // <pic:nvPicPr>
        pugi::xml_node nvPicPr_node = pic_node.append_child("pic:nvPicPr");
        // <pic:cNvPr id="0" name="..."/>
        pugi::xml_node cNvPr_node = nvPicPr_node.append_child("pic:cNvPr");
        cNvPr_node.append_attribute("id").set_value("0");
        std::string filename = m_path.substr(m_path.find_last_of("/\\") + 1);
        cNvPr_node.append_attribute("name").set_value(filename.c_str());
        // <pic:cNvPicPr/>
        nvPicPr_node.append_child("pic:cNvPicPr");

        // <pic:blipFill>
        pugi::xml_node blipFill_node = pic_node.append_child("pic:blipFill");
        // <a:blip r:embed="..."/>
        pugi::xml_node blip_node = blipFill_node.append_child("a:blip");
        blip_node.append_attribute("r:embed").set_value(relationship_id.c_str());
        // <a:stretch> <a:fillRect/> </a:stretch>
        pugi::xml_node stretch_node = blipFill_node.append_child("a:stretch");
        stretch_node.append_child("a:fillRect");

        // <pic:spPr>
        pugi::xml_node spPr_node = pic_node.append_child("pic:spPr");
        // <a:xfrm>
        pugi::xml_node xfrm_node = spPr_node.append_child("a:xfrm");
        // <a:off x="0" y="0"/>
        pugi::xml_node off_node = xfrm_node.append_child("a:off");
        off_node.append_attribute("x").set_value("0");
        off_node.append_attribute("y").set_value("0");
        // <a:ext cx="..." cy="..."/>
        pugi::xml_node ext_node = xfrm_node.append_child("a:ext");
        ext_node.append_attribute("cx").set_value(cx_str.c_str());
        ext_node.append_attribute("cy").set_value(cy_str.c_str());

        // <a:prstGeom prst="rect">
        pugi::xml_node prstGeom_node = spPr_node.append_child("a:prstGeom");
        prstGeom_node.append_attribute("prst").set_value("rect");
        // <a:avLst/>
        prstGeom_node.append_child("a:avLst");
    }
} // namespace duckx
