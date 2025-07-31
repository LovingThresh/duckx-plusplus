# Predefined Style Collections

This directory contains professionally designed style collections for common document types. Each XML file defines a complete style set that can be loaded and applied to documents for consistent, professional formatting.

## Available Style Collections

### 1. Business Document (`business_document.xml`)

**Purpose**: Professional business reports, proposals, and corporate documents  
**Style Set Name**: `BusinessDocument`  
**Theme**: Corporate blue with professional typography

**Key Features**:
- Clean, professional layout with corporate blue accent color (#1F4E79)
- Calibri font family for modern readability
- Structured hierarchy with clear section differentiation
- Financial table styling for reports and proposals
- Executive summary and highlight formatting

**Best For**:
- Annual reports
- Business proposals
- Executive summaries
- Financial documents
- Corporate presentations

**Sample Usage**:
```cpp
XmlStyleParser parser;
auto styles = parser.load_styles_from_file_safe("docs/predefined_styles/business_document.xml");
StyleManager& mgr = document.get_style_manager();
for (auto& style : styles.value()) {
    mgr.register_style_safe(std::move(style));
}
mgr.apply_style_set_safe("BusinessDocument", document);
```

### 2. Technical Documentation (`technical_documentation.xml`)

**Purpose**: API documentation, software manuals, and technical guides  
**Style Set Name**: `TechnicalDocumentation`  
**Theme**: Code-focused with syntax highlighting and technical typography

**Key Features**:
- Segoe UI and Fira Code fonts for technical content
- Code block styling with syntax highlighting colors
- Function signature and parameter formatting
- Warning and note callouts
- API reference table layouts
- Inline code and technical term styling

**Best For**:
- API documentation
- Software user manuals
- Technical specifications
- Code documentation
- Developer guides

**Sample Usage**:
```cpp
// Load and apply technical documentation styles
auto tech_styles = parser.load_styles_from_file_safe("docs/predefined_styles/technical_documentation.xml");
// Register styles and apply the TechnicalDocumentation style set
```

### 3. Academic Paper (`academic_paper.xml`)

**Purpose**: Research papers, theses, and academic publications  
**Style Set Name**: `AcademicPaper`  
**Theme**: IEEE/ACM style formatting with proper academic typography

**Key Features**:
- Times New Roman font for academic tradition
- Proper abstract and keyword formatting
- Figure and table caption styling
- Citation and reference list formatting
- Double-spaced body text with proper indentation
- Footnote and equation styling

**Best For**:
- Research papers
- Conference submissions
- Academic theses
- Journal articles
- Scientific reports

**Formatting Standards**:
- Double-spaced body text (2.0 line spacing)
- 36pt first-line paragraph indentation
- Centered abstract with 36pt margins
- IEEE-style section numbering
- Proper citation formatting

### 4. Modern Report (`modern_report.xml`)

**Purpose**: Contemporary reports with clean, minimal design  
**Style Set Name**: `ModernReport`  
**Theme**: Modern typography with subtle colors and clean layouts

**Key Features**:
- Montserrat and Inter fonts for contemporary feel
- Large statistical number display
- Callout boxes and key point formatting
- Quote styling with attribution
- Minimal table designs
- Subtle color palette with blue accents

**Best For**:
- Market research reports
- Data analysis reports
- Contemporary business documents
- Design-focused presentations
- Modern corporate communications

**Design Philosophy**:
- Clean, minimal aesthetic
- Emphasis on readability
- Strategic use of whitespace
- Modern color palette
- Typography-driven hierarchy

## Usage Instructions

### Loading Individual Styles

```cpp
#include "XmlStyleParser.hpp"
#include "StyleManager.hpp"

// Load specific style collection
XmlStyleParser parser;
auto result = parser.load_styles_from_file_safe("docs/predefined_styles/business_document.xml");

if (result.ok()) {
    // Register individual styles
    StyleManager& style_mgr = document.get_style_manager();
    for (auto& style : result.value()) {
        auto register_result = style_mgr.register_style_safe(std::move(style));
        if (!register_result.ok()) {
            std::cerr << "Failed to register style: " << register_result.error().to_string() << std::endl;
        }
    }
}
```

### Loading Style Sets

```cpp
// Load and apply complete style set
auto set_result = parser.load_style_sets_from_file_safe("docs/predefined_styles/modern_report.xml");
if (set_result.ok()) {
    for (const auto& style_set : set_result.value()) {
        auto register_result = style_mgr.register_style_set_safe(style_set);
        if (register_result.ok()) {
            // Apply the style set to the entire document
            auto apply_result = style_mgr.apply_style_set_safe(style_set.name, document);
            if (!apply_result.ok()) {
                std::cerr << "Failed to apply style set: " << apply_result.error().to_string() << std::endl;
            }
        }
    }
}
```

### Applying Individual Styles

```cpp
// Apply specific styles to document elements
auto body_result = document.get_body_safe();
if (body_result.ok()) {
    Body& body = body_result.value();
    
    // Add and style a title
    auto title_para = body.add_paragraph_safe("Document Title");
    if (title_para.ok()) {
        style_mgr.apply_style_safe("Business_Title", title_para.value());
    }
    
    // Add and style body text
    auto body_para = body.add_paragraph_safe("This is the document content...");
    if (body_para.ok()) {
        style_mgr.apply_style_safe("Business_BodyText", body_para.value());
    }
    
    // Add and style a table
    auto table_result = body.add_table_safe(3, 4);
    if (table_result.ok()) {
        style_mgr.apply_style_safe("Business_Table", table_result.value());
    }
}
```

## Customization

All style collections can be customized by:

1. **Modifying XML files** - Edit color codes, font names, spacing values
2. **Creating derived styles** - Use existing styles as base styles
3. **Mixing style sets** - Combine styles from different collections
4. **Adding custom styles** - Extend collections with organization-specific styles

### Example Customization

```xml
<!-- Add custom style based on existing business style -->
<Style name="Company_Title" type="mixed" base="Business_Title">
    <Character>
        <Color>#FF6B35</Color>  <!-- Company orange color -->
    </Character>
</Style>
```

## Font Requirements

Some styles reference specific fonts that may need to be installed:

- **Montserrat**: Modern geometric sans-serif (Google Fonts)
- **Fira Code**: Programming font with ligatures (Google Fonts)
- **Inter**: Modern sans-serif optimized for UI (Google Fonts)
- **Segoe UI**: Standard Windows system font
- **Times New Roman**: Standard serif font (widely available)
- **Calibri**: Microsoft Office default font

## Validation

All XML files have been validated against the DuckX style schema (version 1.0). They include:

- Proper namespace declarations
- Valid style property combinations
- Consistent color and measurement formats
- Complete style set definitions

## Contributing

To contribute new style collections:

1. Follow the existing XML structure
2. Use semantic style names with collection prefixes
3. Include comprehensive style sets
4. Test with sample documents
5. Document the intended use case and design philosophy

## Support

For issues with predefined styles:

1. Check XML validation against schema
2. Verify font availability on target systems
3. Test style application in isolated environments
4. Report issues with specific style names and expected vs actual behavior

Each style collection includes extensive documentation within the XML comments and comprehensive style coverage for their respective document types.