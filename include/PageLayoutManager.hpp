/*!
 * @file PageLayoutManager.hpp
 * @brief Page layout and section management for DOCX documents
 * 
 * Provides comprehensive page layout control including margins, orientation,
 * page size, headers/footers, and section breaks. Manages document sections
 * and page formatting properties.
 * 
 * @date 2025.07
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "duckx_export.h"
#include "Error.hpp"
#include "constants.hpp"
#include "pugixml.hpp"

namespace duckx {
    
    // Forward declarations
    class Document;
    class HeaderFooterManager;
    
    /*!
     * @brief Page orientation options
     */
    enum class DUCKX_API PageOrientation {
        PORTRAIT,    //!< Portrait orientation (default)
        LANDSCAPE    //!< Landscape orientation
    };
    
    /*!
     * @brief Standard page sizes
     */
    enum class DUCKX_API PageSize {
        A4,          //!< A4 (210 x 297 mm)
        LETTER,      //!< US Letter (8.5 x 11 inches)
        LEGAL,       //!< US Legal (8.5 x 14 inches)
        A3,          //!< A3 (297 x 420 mm)
        A5,          //!< A5 (148 x 210 mm)
        CUSTOM       //!< Custom size (requires width/height)
    };
    
    /*!
     * @brief Section break types
     */
    enum class DUCKX_API SectionBreakType {
        NEXT_PAGE,    //!< Break to next page
        EVEN_PAGE,    //!< Break to next even page
        ODD_PAGE,     //!< Break to next odd page
        CONTINUOUS,   //!< Continuous section break
        COLUMN        //!< Column break
    };
    
    /*!
     * @brief Page margins configuration
     */
    struct DUCKX_API PageMargins {
        double top_mm = 25.4;      //!< Top margin in millimeters
        double bottom_mm = 25.4;   //!< Bottom margin in millimeters
        double left_mm = 25.4;     //!< Left margin in millimeters
        double right_mm = 25.4;    //!< Right margin in millimeters
        double header_mm = 12.7;   //!< Header margin in millimeters
        double footer_mm = 12.7;   //!< Footer margin in millimeters
        
        PageMargins() = default;
        PageMargins(double all_margins_mm) 
            : top_mm(all_margins_mm), bottom_mm(all_margins_mm),
              left_mm(all_margins_mm), right_mm(all_margins_mm) {}
    };
    
    /*!
     * @brief Page size configuration
     */
    struct DUCKX_API PageSizeConfig {
        PageSize size = PageSize::A4;  //!< Standard page size
        double width_mm = 210.0;       //!< Width in millimeters (for custom)
        double height_mm = 297.0;      //!< Height in millimeters (for custom)
        PageOrientation orientation = PageOrientation::PORTRAIT;
        
        PageSizeConfig() = default;
        PageSizeConfig(PageSize std_size, PageOrientation orient = PageOrientation::PORTRAIT)
            : size(std_size), orientation(orient) {
            apply_standard_size();
        }
        
    private:
        void apply_standard_size();
    };
    
    /*!
     * @brief Section properties configuration
     */
    struct DUCKX_API SectionProperties {
        PageSizeConfig page_config;        //!< Page size and orientation
        PageMargins margins;               //!< Page margins
        bool different_first_page = false; //!< Different header/footer for first page
        bool different_odd_even = false;   //!< Different headers/footers for odd/even pages
        int columns = 1;                   //!< Number of columns
        double column_spacing_mm = 12.7;   //!< Spacing between columns
        bool vertical_alignment_center = false; //!< Center content vertically
        
        SectionProperties() = default;
    };
    
    /*!
     * @brief Document section representation
     */
    class DUCKX_API DocumentSection {
    public:
        DocumentSection(pugi::xml_node section_node);
        ~DocumentSection() = default;
        
        // Section properties
        Result<void> set_properties_safe(const SectionProperties& props);
        Result<SectionProperties> get_properties_safe() const;
        
        // Margins
        Result<void> set_margins_safe(const PageMargins& margins);
        Result<PageMargins> get_margins_safe() const;
        
        // Page size and orientation
        Result<void> set_page_size_safe(const PageSizeConfig& config);
        Result<PageSizeConfig> get_page_size_safe() const;
        
        // Columns
        Result<void> set_columns_safe(int column_count, double spacing_mm = 12.7);
        Result<int> get_column_count_safe() const;
        
        // Header/Footer settings
        Result<void> set_different_first_page_safe(bool different);
        Result<void> set_different_odd_even_safe(bool different);
        Result<bool> has_different_first_page_safe() const;
        Result<bool> has_different_odd_even_safe() const;
        
    private:
        pugi::xml_node m_section_node;
        
        Result<pugi::xml_node> get_or_create_page_size_node_safe();
        Result<pugi::xml_node> get_or_create_margins_node_safe();
        Result<pugi::xml_node> get_or_create_columns_node_safe();
    };
    
    /*!
     * @brief Manager for page layout and document sections
     * 
     * Provides comprehensive page layout management including margins,
     * page size, orientation, and section breaks.
     * 
     * **Example usage:**
     * @code
     * auto& layout = doc.page_layout();
     * layout.set_margins_safe(PageMargins(20.0)); // 20mm all around
     * layout.set_page_size_safe(PageSizeConfig(PageSize::A4, PageOrientation::LANDSCAPE));
     * layout.insert_section_break_safe(SectionBreakType::NEXT_PAGE);
     * @endcode
     */
    class DUCKX_API PageLayoutManager {
    public:
        /*!
         * @brief Construct page layout manager for a document
         * @param doc Parent document
         * @param doc_xml Document XML for section manipulation
         */
        PageLayoutManager(Document* doc, pugi::xml_document* doc_xml);
        ~PageLayoutManager() = default;
        
        // Disable copy, enable move
        PageLayoutManager(const PageLayoutManager&) = delete;
        PageLayoutManager& operator=(const PageLayoutManager&) = delete;
        PageLayoutManager(PageLayoutManager&&) noexcept = default;
        PageLayoutManager& operator=(PageLayoutManager&&) noexcept = default;
        
        // ---- Global Page Settings ----
        
        /*!
         * @brief Set page margins for the current section
         * @param margins Margin configuration
         * @return Result indicating success or error
         */
        Result<void> set_margins_safe(const PageMargins& margins);
        
        /*!
         * @brief Get current page margins
         * @return Result containing margin configuration or error
         */
        Result<PageMargins> get_margins_safe() const;
        
        /*!
         * @brief Set page size and orientation
         * @param config Page size configuration
         * @return Result indicating success or error
         */
        Result<void> set_page_size_safe(const PageSizeConfig& config);
        
        /*!
         * @brief Get current page size configuration
         * @return Result containing page size config or error
         */
        Result<PageSizeConfig> get_page_size_safe() const;
        
        /*!
         * @brief Set page orientation
         * @param orientation Page orientation (portrait/landscape)
         * @return Result indicating success or error
         */
        Result<void> set_orientation_safe(PageOrientation orientation);
        
        /*!
         * @brief Get current page orientation
         * @return Result containing orientation or error
         */
        Result<PageOrientation> get_orientation_safe() const;
        
        // ---- Column Layout ----
        
        /*!
         * @brief Set column layout for current section
         * @param column_count Number of columns (1-10)
         * @param spacing_mm Spacing between columns in millimeters
         * @return Result indicating success or error
         */
        Result<void> set_columns_safe(int column_count, double spacing_mm = 12.7);
        
        /*!
         * @brief Get current column configuration
         * @return Result containing column count or error
         */
        Result<int> get_column_count_safe() const;
        
        // ---- Section Management ----
        
        /*!
         * @brief Insert a section break at current position
         * @param break_type Type of section break
         * @return Result indicating success or error
         */
        Result<void> insert_section_break_safe(SectionBreakType break_type);
        
        /*!
         * @brief Get all document sections
         * @return Result containing vector of sections or error
         */
        Result<std::vector<DocumentSection>> get_all_sections_safe() const;
        
        /*!
         * @brief Get current active section
         * @return Result containing current section or error
         */
        Result<DocumentSection> get_current_section_safe() const;
        
        /*!
         * @brief Get section by index
         * @param section_index Zero-based section index
         * @return Result containing section or error
         */
        Result<DocumentSection> get_section_safe(int section_index) const;
        
        /*!
         * @brief Get total number of sections
         * @return Number of sections in document
         */
        int get_section_count() const;
        
        // ---- Header/Footer Page Settings ----
        
        /*!
         * @brief Enable different first page headers/footers
         * @param different True to enable different first page
         * @return Result indicating success or error
         */
        Result<void> set_different_first_page_safe(bool different);
        
        /*!
         * @brief Enable different odd/even page headers/footers
         * @param different True to enable different odd/even pages
         * @return Result indicating success or error
         */
        Result<void> set_different_odd_even_safe(bool different);
        
        /*!
         * @brief Check if different first page is enabled
         * @return Result containing boolean or error
         */
        Result<bool> has_different_first_page_safe() const;
        
        /*!
         * @brief Check if different odd/even pages is enabled
         * @return Result containing boolean or error
         */
        Result<bool> has_different_odd_even_safe() const;
        
        // ---- Page Numbering ----
        
        /*!
         * @brief Set page numbering format for current section
         * @param format Numbering format (decimal, roman, etc.)
         * @param start_number Starting page number
         * @return Result indicating success or error
         */
        Result<void> set_page_numbering_safe(
            const std::string& format = "decimal", 
            int start_number = 1);
        
        /*!
         * @brief Insert page number field at current position
         * @param format Display format for page number
         * @return Result indicating success or error
         */
        Result<void> insert_page_number_safe(const std::string& format = "");
        
        // ---- Vertical Alignment ----
        
        /*!
         * @brief Set vertical alignment for page content
         * @param center_vertically True to center content vertically
         * @return Result indicating success or error
         */
        Result<void> set_vertical_alignment_safe(bool center_vertically);
        
        // ---- Utility Functions ----
        
        /*!
         * @brief Apply section properties to current section
         * @param props Complete section properties
         * @return Result indicating success or error
         */
        Result<void> apply_section_properties_safe(const SectionProperties& props);
        
        /*!
         * @brief Get complete section properties for current section
         * @return Result containing section properties or error
         */
        Result<SectionProperties> get_section_properties_safe() const;
        
        /*!
         * @brief Convert millimeters to Word units (twips)
         * @param mm Millimeters
         * @return Twips (1/20th of a point)
         */
        static int mm_to_twips(double mm);
        
        /*!
         * @brief Convert Word units (twips) to millimeters
         * @param twips Twips (1/20th of a point)
         * @return Millimeters
         */
        static double twips_to_mm(int twips);
        
    private:
        Document* m_document;
        pugi::xml_document* m_doc_xml;
        
        // Helper methods
        Result<pugi::xml_node> get_current_section_pr_safe() const;
        Result<pugi::xml_node> create_section_pr_safe();
        Result<pugi::xml_node> get_or_create_page_size_node_safe(pugi::xml_node sect_pr);
        Result<pugi::xml_node> get_or_create_page_margins_node_safe(pugi::xml_node sect_pr);
        Result<pugi::xml_node> get_or_create_columns_node_safe(pugi::xml_node sect_pr);
        
        static std::string section_break_type_to_string(SectionBreakType type);
        static std::string page_orientation_to_string(PageOrientation orientation);
        static PageOrientation string_to_page_orientation(const std::string& str);
    };
    
} // namespace duckx