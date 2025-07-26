/*!
 * @file DrawingElement.cpp
 * @brief Implementation of drawable element base functionality
 * 
 * Provides common drawing operations including coordinate conversion,
 * sizing, positioning, and base XML generation functionality.
 */
#include "DrawingElement.hpp"

namespace duckx
{
    // 1 inch = 914400 EMUs. Common screen DPI is 96.
    /*! @brief Convert pixels to EMU using standard 96 DPI */
    long long pixels_to_emu(const int pixels)
    {
        if (pixels <= 0)
            return 0;
        constexpr int dpi = 96;
        return static_cast<long long>(pixels) * 914400 / dpi;
    }

    void DrawingElement::set_size(const int width_px, const int height_px)
    {
        m_width_emu = pixels_to_emu(width_px);
        m_height_emu = pixels_to_emu(height_px);
    }

    void DrawingElement::set_absolute_position(const long long x_emu, const long long y_emu, const RelativeFrom h_rel,
                                               const RelativeFrom v_rel)
    {
        m_pos_x_emu = x_emu;
        m_pos_y_emu = y_emu;
        m_h_relative_from = h_rel;
        m_v_relative_from = v_rel;
        m_has_position = true;
    }
} // namespace duckx
