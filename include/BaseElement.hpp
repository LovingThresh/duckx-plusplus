/*!
 * @file BaseElement.hpp
 * @brief Main header for DOCX document element hierarchy
 * 
 * This is the primary include file for all base element classes.
 * It includes all necessary component headers to provide the complete
 * document element API while maintaining organized code structure.
 * 
 * Usage: Include this file to access all document element types:
 * - DocxElement (base class)
 * - Run (text runs)
 * - Paragraph (paragraphs)
 * - Table, TableRow, TableCell (table elements)
 * 
 * @date 2025.07
 */
#pragma once

// Include all component headers
#include "BaseElement_Core.hpp"
#include "BaseElement_Run.hpp"
#include "BaseElement_Paragraph.hpp"
#include "BaseElement_Table.hpp"

// Forward declarations for external dependencies
namespace duckx
{
    class Document;
    class StyleManager;
}
