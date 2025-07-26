/*!
 * @file Image.cpp
 * @brief Implementation of image embedding functionality
 * 
 * Handles image file loading, processing, sizing, and XML generation
 * for embedding images into DOCX documents.
 */
#include "Image.hpp"

#include <iostream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace duckx
{
    std::string relative_from_to_string(RelativeFrom rel)
    {
        switch (rel)
        {
            case RelativeFrom::PAGE:
                return "page";
            case RelativeFrom::MARGIN:
                return "margin";
            default:
                return "page";
        }
    }

    Image::Image(std::string path, const int max_width_px) : m_path(std::move(path))
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

    // This is where the complex XML generation is now cleanly encapsulated.
    void Image::generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
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

        // Choose between inline and anchor based on whether absolute position is set
        if (m_has_position)
        {
            // Generate anchor (absolute positioning) XML
            generate_anchor_xml(drawing_node, relationship_id, drawing_id, cx_str, cy_str, docpr_id_str);
        }
        else
        {
            // Generate inline (default) XML
            generate_inline_xml(drawing_node, relationship_id, drawing_id, cx_str, cy_str, docpr_id_str);
        }
    }

    void Image::generate_inline_xml(pugi::xml_node drawing_node, const std::string& relationship_id,
                                   const unsigned int drawing_id, const std::string& cx_str,
                                   const std::string& cy_str, const std::string& docpr_id_str) const
    {
        // <wp:inline distT="0" distB="0" distL="0" distR="0">
        pugi::xml_node inline_node = drawing_node.append_child("wp:inline");
        inline_node.append_attribute("distT").set_value("0");
        inline_node.append_attribute("distB").set_value("0");
        inline_node.append_attribute("distL").set_value("0");
        inline_node.append_attribute("distR").set_value("0");

        // Add common content (extent, docPr, graphic, etc.)
        add_common_drawing_content(inline_node, relationship_id, drawing_id, cx_str, cy_str, docpr_id_str);
    }

    void Image::generate_anchor_xml(pugi::xml_node drawing_node, const std::string& relationship_id,
                                   const unsigned int drawing_id, const std::string& cx_str,
                                   const std::string& cy_str, const std::string& docpr_id_str) const
    {
        // <wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="251658240"
        //           behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
        pugi::xml_node anchor_node = drawing_node.append_child("wp:anchor");
        anchor_node.append_attribute("distT").set_value("0");
        anchor_node.append_attribute("distB").set_value("0");
        anchor_node.append_attribute("distL").set_value("0");
        anchor_node.append_attribute("distR").set_value("0");
        anchor_node.append_attribute("simplePos").set_value("0");
        anchor_node.append_attribute("relativeHeight").set_value("251658240");
        anchor_node.append_attribute("behindDoc").set_value("0");
        anchor_node.append_attribute("locked").set_value("0");
        anchor_node.append_attribute("layoutInCell").set_value("1");
        anchor_node.append_attribute("allowOverlap").set_value("1");

        // <wp:simplePos x="0" y="0"/>
        pugi::xml_node simple_pos_node = anchor_node.append_child("wp:simplePos");
        simple_pos_node.append_attribute("x").set_value("0");
        simple_pos_node.append_attribute("y").set_value("0");

        // <wp:positionH relativeFrom="...">
        pugi::xml_node pos_h_node = anchor_node.append_child("wp:positionH");
        pos_h_node.append_attribute("relativeFrom").set_value(relative_from_to_string(m_h_relative_from).c_str());
        pugi::xml_node pos_h_offset = pos_h_node.append_child("wp:posOffset");
        pos_h_offset.text().set(std::to_string(m_pos_x_emu).c_str());

        // <wp:positionV relativeFrom="...">
        pugi::xml_node pos_v_node = anchor_node.append_child("wp:positionV");
        pos_v_node.append_attribute("relativeFrom").set_value(relative_from_to_string(m_v_relative_from).c_str());
        pugi::xml_node pos_v_offset = pos_v_node.append_child("wp:posOffset");
        pos_v_offset.text().set(std::to_string(m_pos_y_emu).c_str());

        // Add common content (extent, docPr, graphic, etc.)
        add_common_drawing_content(anchor_node, relationship_id, drawing_id, cx_str, cy_str, docpr_id_str);
    }

    void Image::add_common_drawing_content(pugi::xml_node container_node, const std::string& relationship_id,
                                          const unsigned int drawing_id, const std::string& cx_str,
                                          const std::string& cy_str, const std::string& docpr_id_str) const
    {
        // <wp:extent cx="..." cy="...">
        pugi::xml_node extent_node = container_node.append_child("wp:extent");
        extent_node.append_attribute("cx").set_value(cx_str.c_str());
        extent_node.append_attribute("cy").set_value(cy_str.c_str());

        // <wp:effectExtent l="0" t="0" r="0" b="0"/>
        pugi::xml_node effect_extent_node = container_node.append_child("wp:effectExtent");
        effect_extent_node.append_attribute("l").set_value("0");
        effect_extent_node.append_attribute("t").set_value("0");
        effect_extent_node.append_attribute("r").set_value("0");
        effect_extent_node.append_attribute("b").set_value("0");

        // <wp:docPr id="..." name="..."/>
        pugi::xml_node docPr_node = container_node.append_child("wp:docPr");
        docPr_node.append_attribute("id").set_value(docpr_id_str.c_str());
        docPr_node.append_attribute("name").set_value(("Picture " + docpr_id_str).c_str());

        // <wp:cNvGraphicFramePr>
        pugi::xml_node cNvGraphicFramePr_node = container_node.append_child("wp:cNvGraphicFramePr");
        pugi::xml_node graphicFrameLocks_node = cNvGraphicFramePr_node.append_child("a:graphicFrameLocks");
        graphicFrameLocks_node.append_attribute("noChangeAspect").set_value("1");

        // <a:graphic>
        pugi::xml_node graphic_node = container_node.append_child("a:graphic");

        // <a:graphicData uri="...">
        pugi::xml_node graphicData_node = graphic_node.append_child("a:graphicData");
        graphicData_node.append_attribute("uri").set_value("http://schemas.openxmlformats.org/drawingml/2006/picture");

        // <pic:pic>
        pugi::xml_node pic_node = graphicData_node.append_child("pic:pic");

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

    const std::string& Image::get_path() const
    {
        return m_path;
    }
} // namespace duckx
