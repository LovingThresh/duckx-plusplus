/*
 * @file: DrawingElement.hpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.08
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */

#pragma once

#include <pugixml.hpp>
#include <string>

namespace duckx
{
    long long pixels_to_emu(int pixels);
    enum class RelativeFrom {
        PAGE,
        MARGIN
    };

    class DrawingElement
    {
    public:
        virtual ~DrawingElement() = default;

        void set_size(int width_px, int height_px);
        void set_absolute_position(long long x_emu, long long y_emu,
                                           RelativeFrom h_rel = RelativeFrom::PAGE,
                                           RelativeFrom v_rel = RelativeFrom::PAGE);

        virtual void generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                          unsigned int drawing_id) const = 0;

    protected:
        DrawingElement() = default;

        long long m_width_emu = 0;
        long long m_height_emu = 0;

        bool m_has_position = false;
        long long m_pos_x_emu = 0;
        long long m_pos_y_emu = 0;
        RelativeFrom m_h_relative_from = RelativeFrom::PAGE;
        RelativeFrom m_v_relative_from = RelativeFrom::PAGE;
    };
} // namespace duckx
