/*!
 * @file DrawingElement.hpp
 * @brief Base class for drawable elements in DOCX documents
 * 
 * Provides common functionality for elements that can be drawn in documents,
 * including sizing, positioning, and XML generation interfaces.
 */

#pragma once

#include <string>
#include "duckx_export.h"
#include "pugixml.hpp"

namespace duckx
{
    /*! @brief Convert pixels to EMU (English Metric Units) */
    long long pixels_to_emu(int pixels);
    /*! @brief Reference point for positioning elements */
    enum class DUCKX_API RelativeFrom {
        PAGE,   //!< Position relative to page edges
        MARGIN  //!< Position relative to page margins
    };

    /*!
     * @brief Base class for drawable elements
     * 
     * Abstract base class for elements that can be drawn in DOCX documents
     * such as images and text boxes. Provides common sizing and positioning
     * functionality along with XML generation interface.
     */
    class DUCKX_API DrawingElement
    {
    public:
        virtual ~DrawingElement() = default;

        /*! @brief Set the size of the drawing element in pixels */
        void set_size(int width_px, int height_px);
        /*! @brief Set absolute position in EMU units */
        void set_absolute_position(long long x_emu, long long y_emu,
                                           RelativeFrom h_rel = RelativeFrom::PAGE,
                                           RelativeFrom v_rel = RelativeFrom::PAGE);

        /*! @brief Generate XML structure for this drawing element */
        virtual void generate_drawing_xml(pugi::xml_node parent_run_node, const std::string& relationship_id,
                                          unsigned int drawing_id) const = 0;

    protected:
        DrawingElement() = default;

        long long m_width_emu = 0;  //!< Width in EMU units
        long long m_height_emu = 0; //!< Height in EMU units

        bool m_has_position = false;                         //!< Whether position is set
        long long m_pos_x_emu = 0;                          //!< X position in EMU
        long long m_pos_y_emu = 0;                          //!< Y position in EMU
        RelativeFrom m_h_relative_from = RelativeFrom::PAGE; //!< Horizontal reference point
        RelativeFrom m_v_relative_from = RelativeFrom::PAGE; //!< Vertical reference point
    };
} // namespace duckx
