/*!
 * @file Body.cpp
 * @brief Implementation of document body content management
 * 
 * Handles the main content area operations including paragraph and table
 * creation, iteration, and the modern Result<T> API implementations.
 */
#include "../include/Body.hpp"

namespace duckx
{
    Body::Body(const pugi::xml_node bodyNode)
        : m_bodyNode(bodyNode)
    {
        if (m_bodyNode)
        {
            m_paragraph.set_parent(m_bodyNode);
            m_paragraph.set_current(m_bodyNode.child("w:p"));
            m_table.set_parent(m_bodyNode);
            m_table.set_current(m_bodyNode.child("w:tbl"));
        }
    }

    absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> Body::paragraphs()
    {
        if (m_bodyNode)
        {
            m_paragraph.set_current(m_bodyNode.child("w:p"));
        }
        else
        {
            m_paragraph.set_current(pugi::xml_node());
        }

        m_paragraph.set_parent(m_bodyNode);

        return make_element_range(m_paragraph);
    }

    absl::enable_if_t<is_docx_element<Paragraph>::value, ElementRange<Paragraph>> Body::paragraphs() const
    {
        Paragraph temp_para;

        if (m_bodyNode)
        {
            temp_para.set_current(m_bodyNode.child("w:p"));
        }
        else
        {
            temp_para.set_current(pugi::xml_node());
        }
        temp_para.set_parent(m_bodyNode);

        return make_element_range(temp_para);
    }

    absl::enable_if_t<is_docx_element<Table>::value, ElementRange<Table>> Body::tables()
    {
        if (m_bodyNode)
        {
            m_table.set_current(m_bodyNode.child("w:tbl"));
        }
        else
        {
            m_table.set_current(pugi::xml_node());
        }

        m_table.set_parent(m_bodyNode);

        return make_element_range(m_table);
    }

    absl::enable_if_t<is_docx_element<Table>::value, ElementRange<Table>> Body::tables() const
    {
        Table temp_table;
        if (m_bodyNode)
        {
            temp_table.set_current(m_bodyNode.child("w:tbl"));
        }
        else
        {
            temp_table.set_current(pugi::xml_node());
        }

        temp_table.set_parent(m_bodyNode);

        return make_element_range(temp_table);
    }

    Paragraph Body::add_paragraph(const std::string& text, const formatting_flag f)
    {
        const pugi::xml_node pNode = m_bodyNode.append_child("w:p");
        Paragraph newPara(m_bodyNode, pNode);
        if (!text.empty())
        {
            newPara.add_run(text, f);
        }
        return newPara;
    }

    Table Body::add_table(const int rows, const int cols)
    {
        // Create table structure following DOCX specification requirements
        pugi::xml_node new_tbl_node = m_bodyNode.append_child("w:tbl");

        // Add table properties with default borders for visibility
        pugi::xml_node tbl_pr_node = new_tbl_node.append_child("w:tblPr");
        pugi::xml_node tbl_borders_node = tbl_pr_node.append_child("w:tblBorders");
        tbl_borders_node.append_child("w:top").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:left").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:bottom").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:right").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:insideH").append_attribute("w:val").set_value("single");
        tbl_borders_node.append_child("w:insideV").append_attribute("w:val").set_value("single");

        // Define table grid structure with column widths
        pugi::xml_node tbl_grid_node = new_tbl_node.append_child("w:tblGrid");
        for (int i = 0; i < cols; ++i)
        {
            // Set default column width (2390 twips ≈ 4.2cm)
            tbl_grid_node.append_child("w:gridCol").append_attribute("w:w").set_value("2390");
        }

        // Create table rows and cells with required paragraph nodes
        for (int r = 0; r < rows; ++r)
        {
            pugi::xml_node tr_node = new_tbl_node.append_child("w:tr");
            for (int c = 0; c < cols; ++c)
            {
                pugi::xml_node tc_node = tr_node.append_child("w:tc");
                // Each cell must contain at least one paragraph (DOCX requirement)
                tc_node.append_child("w:p");
            }
        }

        // Return Table wrapper for the created XML structure
        return {m_bodyNode, new_tbl_node};
    }

    Result<Paragraph> Body::add_paragraph_safe(const std::string& text, const formatting_flag f)
    {
        try {
            // Validate body node
            if (!m_bodyNode) {
                return Result<Paragraph>(Error(ErrorCategory::ELEMENT_OPERATION, 
                           ErrorCode::ELEMENT_INVALID_STATE,
                           "Body node is invalid or null",
                           DUCKX_ERROR_CONTEXT_OP("add_paragraph_safe")));
            }

            // Validate text length (reasonable limit)
            if (text.length() > 1000000) { // 1MB text limit
                return Result<Paragraph>(Error(ErrorCategory::VALIDATION,
                           ErrorCode::INVALID_TEXT_LENGTH,
                           "Text length exceeds maximum allowed size",
                           DUCKX_ERROR_CONTEXT_OP("add_paragraph_safe")
                           .with_info("text_length", std::to_string(text.length()))
                           .with_info("max_length", "1000000")));
            }

            // Create paragraph node
            const pugi::xml_node pNode = m_bodyNode.append_child("w:p");
            if (!pNode) {
                return Result<Paragraph>(Error(ErrorCategory::ELEMENT_OPERATION,
                           ErrorCode::PARAGRAPH_CREATION_FAILED,
                           "Failed to create paragraph XML node",
                           DUCKX_ERROR_CONTEXT_OP("add_paragraph_safe")));
            }

            // Create paragraph object
            Paragraph newPara(m_bodyNode, pNode);
            
            // Add text content if provided
            if (!text.empty()) {
                try {
                    newPara.add_run(text, f);
                } catch (const std::exception& e) {
                    return Result<Paragraph>(Error(ErrorCategory::ELEMENT_OPERATION,
                               ErrorCode::RUN_CREATION_FAILED,
                               absl::StrFormat("Failed to add run to paragraph: %s", e.what()),
                               DUCKX_ERROR_CONTEXT_OP("add_paragraph_safe")
                               .with_info("text_length", std::to_string(text.length()))));
                }
            }

            return Result<Paragraph>(std::move(newPara));
        }
        catch (const std::exception& e) {
            return Result<Paragraph>(Error(ErrorCategory::ELEMENT_OPERATION,
                       ErrorCode::ELEMENT_OPERATION_FAILED,
                       absl::StrFormat("Unexpected error in add_paragraph_safe: %s", e.what()),
                       DUCKX_ERROR_CONTEXT_OP("add_paragraph_safe")));
        }
    }

    Result<Table> Body::add_table_safe(const int rows, const int cols)
    {
        try {
            // Validate body node
            if (!m_bodyNode) {
                return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                           ErrorCode::ELEMENT_INVALID_STATE,
                           "Body node is invalid or null",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")));
            }

            // Validate table dimensions
            if (rows < 0 || cols < 0) {
                return Result<Table>(Error(ErrorCategory::VALIDATION,
                           ErrorCode::INVALID_ARGUMENT,
                           "Table dimensions cannot be negative",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                           .with_info("rows", std::to_string(rows))
                           .with_info("cols", std::to_string(cols))));
            }

            if (rows > 10000 || cols > 1000) { // Reasonable limits
                return Result<Table>(Error(ErrorCategory::VALIDATION,
                           ErrorCode::RESOURCE_LIMIT_EXCEEDED,
                           "Table dimensions exceed maximum allowed size",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                           .with_info("rows", std::to_string(rows))
                           .with_info("cols", std::to_string(cols))
                           .with_info("max_rows", "10000")
                           .with_info("max_cols", "1000")));
            }

            // Create table main node <w:tbl>
            pugi::xml_node new_tbl_node = m_bodyNode.append_child("w:tbl");
            if (!new_tbl_node) {
                return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                           ErrorCode::TABLE_CREATION_FAILED,
                           "Failed to create table XML node",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")));
            }

            // Add table properties <w:tblPr> and default borders
            pugi::xml_node tbl_pr_node = new_tbl_node.append_child("w:tblPr");
            if (!tbl_pr_node) {
                return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                           ErrorCode::TABLE_CREATION_FAILED,
                           "Failed to create table properties node",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")));
            }

            pugi::xml_node tbl_borders_node = tbl_pr_node.append_child("w:tblBorders");
            if (!tbl_borders_node) {
                return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                           ErrorCode::TABLE_CREATION_FAILED,
                           "Failed to create table borders node",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")));
            }

            // Set border properties
            tbl_borders_node.append_child("w:top").append_attribute("w:val").set_value("single");
            tbl_borders_node.append_child("w:left").append_attribute("w:val").set_value("single");
            tbl_borders_node.append_child("w:bottom").append_attribute("w:val").set_value("single");
            tbl_borders_node.append_child("w:right").append_attribute("w:val").set_value("single");
            tbl_borders_node.append_child("w:insideH").append_attribute("w:val").set_value("single");
            tbl_borders_node.append_child("w:insideV").append_attribute("w:val").set_value("single");

            // Define table grid <w:tblGrid>
            pugi::xml_node tbl_grid_node = new_tbl_node.append_child("w:tblGrid");
            if (!tbl_grid_node) {
                return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                           ErrorCode::TABLE_CREATION_FAILED,
                           "Failed to create table grid node",
                           DUCKX_ERROR_CONTEXT_OP("add_table_safe")));
            }

            for (int i = 0; i < cols; ++i) {
                pugi::xml_node grid_col = tbl_grid_node.append_child("w:gridCol");
                if (!grid_col) {
                    return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                               ErrorCode::TABLE_CREATION_FAILED,
                               absl::StrFormat("Failed to create grid column %d", i),
                               DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                               .with_info("column_index", std::to_string(i))));
                }
                grid_col.append_attribute("w:w").set_value("2390");
            }

            // Create rows <w:tr> and cells <w:tc>
            for (int r = 0; r < rows; ++r) {
                pugi::xml_node tr_node = new_tbl_node.append_child("w:tr");
                if (!tr_node) {
                    return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                               ErrorCode::TABLE_CREATION_FAILED,
                               absl::StrFormat("Failed to create table row %d", r),
                               DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                               .with_info("row_index", std::to_string(r))));
                }

                for (int c = 0; c < cols; ++c) {
                    pugi::xml_node tc_node = tr_node.append_child("w:tc");
                    if (!tc_node) {
                        return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                                   ErrorCode::TABLE_CREATION_FAILED,
                                   absl::StrFormat("Failed to create table cell [%d,%d]", r, c),
                                   DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                                   .with_info("row_index", std::to_string(r))
                                   .with_info("col_index", std::to_string(c))));
                    }

                    // Each cell must contain an empty paragraph <w:p>
                    pugi::xml_node p_node = tc_node.append_child("w:p");
                    if (!p_node) {
                        return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                                   ErrorCode::TABLE_CREATION_FAILED,
                                   absl::StrFormat("Failed to create paragraph in cell [%d,%d]", r, c),
                                   DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                                   .with_info("row_index", std::to_string(r))
                                   .with_info("col_index", std::to_string(c))));
                    }
                }
            }

            // Return a Table object pointing to the newly created table
            return Result<Table>(Table{m_bodyNode, new_tbl_node});
        }
        catch (const std::exception& e) {
            return Result<Table>(Error(ErrorCategory::ELEMENT_OPERATION,
                       ErrorCode::ELEMENT_OPERATION_FAILED,
                       absl::StrFormat("Unexpected error in add_table_safe: %s", e.what()),
                       DUCKX_ERROR_CONTEXT_OP("add_table_safe")
                       .with_info("rows", std::to_string(rows))
                       .with_info("cols", std::to_string(cols))));
        }
    }
} // namespace duckx
