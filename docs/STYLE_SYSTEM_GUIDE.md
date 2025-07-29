# DuckX Style System Guide

This comprehensive guide covers the complete style system implementation in DuckX-PlusPlus, including individual styles, style sets, XML-based definitions, and the modern Result<T> API.

## Table of Contents

- [Overview](#overview)
- [Core Components](#core-components)
- [Style Management](#style-management)
- [Style Sets](#style-sets)
- [XML Style Definitions](#xml-style-definitions)
- [API Usage Examples](#api-usage-examples)
- [Error Handling](#error-handling)
- [Best Practices](#best-practices)

## Overview

The DuckX style system provides comprehensive document formatting capabilities through a modern, type-safe API. The system supports:

- **Individual Styles**: Paragraph, character, table, and mixed styles
- **Style Sets**: Collections of related styles for batch application
- **XML Definitions**: User-friendly XML format for style definitions
- **Cascading Application**: Hierarchical style application (tables → paragraphs → characters)
- **Result<T> API**: Modern error handling with detailed context

## Core Components

### StyleManager

The central hub for all style operations, providing both legacy exception-based and modern Result<T> APIs.

```cpp
#include "StyleManager.hpp"

// Create style manager
StyleManager style_manager;

// Register individual styles
auto heading_style = std::make_unique<Style>("Heading1", StyleType::PARAGRAPH);
auto result = style_manager.register_style_safe(std::move(heading_style));

// Apply styles to document elements
auto apply_result = style_manager.apply_style_safe("Heading1", paragraph);
```

### Style Class

Represents individual style definitions with comprehensive property support.

```cpp
#include "Style.hpp"

// Create a mixed style (paragraph + character properties)
auto style = std::make_unique<Style>("CodeBlock", StyleType::MIXED);

// Set paragraph properties
ParagraphStyleProperties para_props;
para_props.alignment = Alignment::LEFT;
para_props.left_indent_pts = 36.0;
para_props.space_before_pts = 6.0;
para_props.space_after_pts = 6.0;
style->set_paragraph_properties_safe(para_props);

// Set character properties
CharacterStyleProperties char_props;
char_props.font_name = "Consolas";
char_props.font_size_pts = 10.0;
char_props.font_color_hex = "333333";
char_props.highlight_color = HighlightColor::LIGHT_GRAY;
char_props.formatting_flags = none; // No special formatting
style->set_character_properties_safe(char_props);
```

### XmlStyleParser

Parses user-friendly XML style definitions into Style objects.

```cpp
#include "XmlStyleParser.hpp"

XmlStyleParser parser;

// Load styles from XML file
auto styles_result = parser.load_styles_from_file_safe("styles.xml");
if (styles_result.ok()) {
    for (auto& style : styles_result.value()) {
        style_manager.register_style_safe(std::move(style));
    }
}

// Load styles from XML string
std::string xml_content = R"(
<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="Emphasis" type="character">
        <Character>
            <Font name="Arial" size="12pt"/>
            <Color>#FF0000</Color>
            <Format bold="true" italic="true"/>
        </Character>
    </Style>
</StyleSheet>)";

auto xml_styles_result = parser.load_styles_from_string_safe(xml_content);
```

## Style Management

### Creating Styles Programmatically

```cpp
// Paragraph style example
auto para_style = std::make_unique<Style>("CustomParagraph", StyleType::PARAGRAPH);

ParagraphStyleProperties para_props;
para_props.alignment = Alignment::BOTH; // Justified
para_props.line_spacing = 1.5;
para_props.space_before_pts = 12.0;
para_props.space_after_pts = 6.0;
para_props.first_line_indent_pts = 18.0;

auto set_result = para_style->set_paragraph_properties_safe(para_props);
if (set_result.ok()) {
    style_manager.register_style_safe(std::move(para_style));
}

// Character style example
auto char_style = std::make_unique<Style>("Highlight", StyleType::CHARACTER);

CharacterStyleProperties char_props;
char_props.highlight_color = HighlightColor::YELLOW;
char_props.formatting_flags = bold | italic;

char_style->set_character_properties_safe(char_props);
style_manager.register_style_safe(std::move(char_style));

// Table style example
auto table_style = std::make_unique<Style>("DataTable", StyleType::TABLE);

TableStyleProperties table_props;
table_props.table_width_pts = 500.0;
table_props.table_alignment = "center";
table_props.border_style = "single";
table_props.border_width_pts = 1.0;
table_props.border_color_hex = "000000";
table_props.cell_padding_pts = 8.0;

table_style->set_table_properties_safe(table_props);
style_manager.register_style_safe(std::move(table_style));
```

### Style Inheritance

```cpp
// Create base style
auto base_style = std::make_unique<Style>("BaseText", StyleType::CHARACTER);
CharacterStyleProperties base_props;
base_props.font_name = "Times New Roman";
base_props.font_size_pts = 12.0;
base_style->set_character_properties_safe(base_props);
style_manager.register_style_safe(std::move(base_style));

// Create derived style
auto derived_style = std::make_unique<Style>("BoldText", StyleType::CHARACTER);
auto inheritance_result = derived_style->set_base_style_safe("BaseText");
if (inheritance_result.ok()) {
    CharacterStyleProperties derived_props;
    derived_props.formatting_flags = bold;
    derived_style->set_character_properties_safe(derived_props);
    style_manager.register_style_safe(std::move(derived_style));
}
```

## Style Sets

Style sets provide powerful batch style management for related formatting themes.

### Creating Style Sets

```cpp
// Create individual styles first
style_manager.register_style_safe(create_heading_style());
style_manager.register_style_safe(create_body_style());
style_manager.register_style_safe(create_code_style());

// Create style set
StyleSet technical_docs("TechnicalDocumentation");
technical_docs.description = "Complete style set for technical documentation";
technical_docs.included_styles = {
    "DocumentTitle",
    "SectionHeading", 
    "BodyText",
    "CodeBlock",
    "DataTable"
};

// Register the style set
auto register_result = style_manager.register_style_set_safe(technical_docs);
```

### Applying Style Sets

```cpp
// Apply entire style set to document with cascading
auto apply_result = style_manager.apply_style_set_safe("TechnicalDocumentation", document);
if (!apply_result.ok()) {
    std::cerr << "Failed to apply style set: " << apply_result.error().to_string() << std::endl;
}

// Check application status
auto status_result = style_manager.get_style_set_status_safe("TechnicalDocumentation");
if (status_result.ok()) {
    const auto& status = status_result.value();
    std::cout << "Applied " << status.applied_count 
              << " out of " << status.total_count << " styles" << std::endl;
    
    if (!status.failed_styles.empty()) {
        std::cout << "Failed styles: ";
        for (const auto& failed : status.failed_styles) {
            std::cout << failed << " ";
        }
        std::cout << std::endl;
    }
}
```

### Cascading Style Application

Style sets apply styles in hierarchical order:

1. **Table Styles**: Applied to all tables in the document
2. **Paragraph Styles**: Applied to all paragraphs (including those in tables)
3. **Character Styles**: Applied to all runs (including those in paragraphs and tables)

```cpp
// Example cascading application
StyleSet business_report("BusinessReport");
business_report.included_styles = {
    "CompanyTable",     // Will be applied to tables first
    "ReportParagraph",  // Then applied to paragraphs
    "HighlightText"     // Finally applied to character runs
};

auto cascade_result = style_manager.apply_style_set_safe("BusinessReport", document);
// All three style types will be applied in the correct order
```

## XML Style Definitions

### Basic XML Structure

```xml
<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <!-- Individual styles -->
    <Style name="StyleName" type="paragraph|character|table|mixed">
        <!-- Style properties -->
    </Style>
    
    <!-- Style sets -->
    <StyleSet name="SetName" description="Optional description">
        <Include>StyleName1</Include>
        <Include>StyleName2</Include>
    </StyleSet>
</StyleSheet>
```

### Paragraph Properties

```xml
<Style name="CustomParagraph" type="paragraph">
    <Paragraph>
        <Alignment>left|center|right|justify</Alignment>
        <SpaceBefore>12pt</SpaceBefore>
        <SpaceAfter>6pt</SpaceAfter>
        <LineSpacing>1.5</LineSpacing>
        <Indentation left="0pt" right="0pt" firstLine="18pt"/>
    </Paragraph>
</Style>
```

### Character Properties

```xml
<Style name="CustomCharacter" type="character">
    <Character>
        <Font name="Arial" size="12pt"/>
        <Color>#FF0000</Color>
        <Highlight>yellow</Highlight>
        <Format bold="true" italic="false" underline="true" 
                strikethrough="false" subscript="false" superscript="false" 
                smallCaps="false" shadow="false"/>
    </Character>
</Style>
```

### Table Properties

```xml
<Style name="CustomTable" type="table">
    <Table>
        <Width>100%</Width>
        <Alignment>center</Alignment>
        <Borders style="single" width="1pt" color="#000000"/>
        <CellPadding>5pt</CellPadding>
    </Table>
</Style>
```

### Mixed Styles

```xml
<Style name="CodeBlock" type="mixed">
    <Paragraph>
        <SpaceBefore>6pt</SpaceBefore>
        <SpaceAfter>6pt</SpaceAfter>
        <Indentation left="36pt"/>
    </Paragraph>
    <Character>
        <Font name="Consolas" size="10pt"/>
        <Color>#333333</Color>
        <Highlight>lightGray</Highlight>
    </Character>
</Style>
```

### Complete Example XML

```xml
<?xml version="1.0" encoding="UTF-8"?>
<StyleSheet xmlns="http://duckx.org/styles" version="1.0">
    <Style name="DocumentTitle" type="mixed">
        <Paragraph>
            <Alignment>center</Alignment>
            <SpaceBefore>0pt</SpaceBefore>
            <SpaceAfter>24pt</SpaceAfter>
        </Paragraph>
        <Character>
            <Font name="Arial" size="24pt"/>
            <Color>#000080</Color>
            <Format bold="true"/>
        </Character>
    </Style>
    
    <Style name="SectionHeading" type="mixed">
        <Paragraph>
            <Alignment>left</Alignment>
            <SpaceBefore>18pt</SpaceBefore>
            <SpaceAfter>12pt</SpaceAfter>
        </Paragraph>
        <Character>
            <Font name="Arial" size="16pt"/>
            <Color>#000080</Color>
            <Format bold="true"/>
        </Character>
    </Style>
    
    <Style name="BodyText" type="paragraph">
        <Paragraph>
            <Alignment>justify</Alignment>
            <SpaceBefore>0pt</SpaceBefore>
            <SpaceAfter>6pt</SpaceAfter>
            <LineSpacing>1.2</LineSpacing>
            <Indentation firstLine="18pt"/>
        </Paragraph>
    </Style>
    
    <StyleSet name="DocumentSet" description="Complete document formatting">
        <Include>DocumentTitle</Include>
        <Include>SectionHeading</Include>
        <Include>BodyText</Include>
    </StyleSet>
</StyleSheet>
```

## API Usage Examples

### Document-Level Style Integration

```cpp
#include "Document.hpp"
#include "StyleManager.hpp"

// Create document and style manager
auto doc_result = Document::create_safe("document.docx");
if (!doc_result.ok()) {
    return handle_error(doc_result.error());
}
Document& doc = doc_result.value();

StyleManager& style_mgr = doc.get_style_manager();

// Load styles from XML
XmlStyleParser parser;
auto styles_result = parser.load_styles_from_file_safe("styles.xml");
if (styles_result.ok()) {
    for (auto& style : styles_result.value()) {
        style_mgr.register_style_safe(std::move(style));
    }
}

// Apply style set to entire document
auto apply_result = style_mgr.apply_style_set_safe("DocumentSet", doc);
if (!apply_result.ok()) {
    std::cerr << "Style application failed: " << apply_result.error().to_string() << std::endl;
}
```

### Element-Specific Styling

```cpp
// Get document body
auto body_result = doc.get_body_safe();
if (!body_result.ok()) return;
Body& body = body_result.value();

// Add styled paragraph
auto para_result = body.add_paragraph_safe("This is a heading");
if (para_result.ok()) {
    Paragraph& para = para_result.value();
    auto style_result = style_mgr.apply_style_safe("SectionHeading", para);
}

// Add styled table
auto table_result = body.add_table_safe(3, 4);
if (table_result.ok()) {
    Table& table = table_result.value();
    auto table_style_result = style_mgr.apply_style_safe("DataTable", table);
}
```

### Advanced Style Queries

```cpp
// List all registered styles
auto styles_list = style_mgr.list_styles_safe();
if (styles_list.ok()) {
    for (const auto& style_name : styles_list.value()) {
        std::cout << "Available style: " << style_name << std::endl;
    }
}

// Get style information
auto style_info = style_mgr.get_style_safe("SectionHeading");
if (style_info.ok()) {
    const Style& style = style_info.value();
    std::cout << "Style type: " << static_cast<int>(style.type()) << std::endl;
    
    if (style.has_character_properties()) {
        const auto& char_props = style.character_properties();
        if (char_props.font_name.has_value()) {
            std::cout << "Font: " << char_props.font_name.value() << std::endl;
        }
    }
}

// Check style set status
auto set_status = style_mgr.get_style_set_status_safe("DocumentSet");
if (set_status.ok()) {
    const auto& status = set_status.value();
    std::cout << "Style set applied: " << (status.applied_count == status.total_count ? "Yes" : "Partial") << std::endl;
}
```

## Error Handling

The style system uses comprehensive Result<T> error handling:

```cpp
// Pattern 1: Direct error checking
auto result = style_mgr.apply_style_safe("NonExistentStyle", paragraph);
if (!result.ok()) {
    const auto& error = result.error();
    std::cerr << "Error: " << error.to_string() << std::endl;
    std::cerr << "Category: " << static_cast<int>(error.category()) << std::endl;
    if (error.context().has_value()) {
        std::cerr << "Context: " << error.context().value().to_string() << std::endl;
    }
}

// Pattern 2: Monadic error handling
auto final_result = style_mgr.register_style_safe(std::move(style))
    .and_then([&](const auto&) { 
        return style_mgr.apply_style_safe("NewStyle", paragraph); 
    })
    .and_then([&](const auto&) {
        return doc.save_safe();
    });

if (!final_result.ok()) {
    handle_error(final_result.error());
}

// Pattern 3: Error recovery
auto apply_result = style_mgr.apply_style_set_safe("PreferredSet", doc);
if (!apply_result.ok()) {
    // Fallback to default style set
    auto fallback_result = style_mgr.apply_style_set_safe("DefaultSet", doc);
    if (!fallback_result.ok()) {
        // Last resort: individual style application
        style_mgr.apply_style_safe("BasicParagraph", doc);
    }
}
```

## Best Practices

### 1. Style Organization

- **Group related styles** into style sets for easier management
- **Use descriptive names** that indicate the style's purpose
- **Establish naming conventions** (e.g., "Document_Title", "Section_Heading")
- **Create base styles** for common properties and inherit from them

### 2. XML Style Definitions

- **Validate XML files** before deployment
- **Use semantic naming** that reflects content structure
- **Document color codes** and their meanings
- **Keep style definitions** in version control

### 3. Error Handling

- **Always check Result<T> return values**
- **Provide meaningful error messages** to users
- **Implement fallback strategies** for critical operations
- **Log style application failures** for debugging

### 4. Performance Considerations

- **Register styles once** at application startup
- **Apply style sets** rather than individual styles when possible
- **Cache frequently used styles**
- **Avoid repeated XML parsing** in performance-critical code

### 5. Maintenance

- **Regularly review** and clean up unused styles
- **Document style purposes** and usage guidelines
- **Test style combinations** for visual consistency
- **Maintain backward compatibility** when updating style definitions

## Integration Examples

### Complete Document Workflow

```cpp
// Complete example: Technical document creation
int main() {
    // 1. Create document
    auto doc_result = Document::create_safe("technical_guide.docx");
    if (!doc_result.ok()) return 1;
    Document& doc = doc_result.value();
    
    // 2. Load styles
    XmlStyleParser parser;
    auto styles_result = parser.load_styles_from_file_safe("technical_styles.xml");
    if (!styles_result.ok()) return 1;
    
    StyleManager& style_mgr = doc.get_style_manager();
    for (auto& style : styles_result.value()) {
        style_mgr.register_style_safe(std::move(style));
    }
    
    // 3. Apply document-wide style set
    auto set_result = style_mgr.apply_style_set_safe("TechnicalDocument", doc);
    
    // 4. Create content
    auto body_result = doc.get_body_safe();
    if (!body_result.ok()) return 1;
    Body& body = body_result.value();
    
    // Add title
    auto title_result = body.add_paragraph_safe("API Reference Guide");
    if (title_result.ok()) {
        style_mgr.apply_style_safe("DocumentTitle", title_result.value());
    }
    
    // Add sections
    auto section_result = body.add_paragraph_safe("Introduction");
    if (section_result.ok()) {
        style_mgr.apply_style_safe("SectionHeading", section_result.value());
    }
    
    auto content_result = body.add_paragraph_safe("This guide provides comprehensive documentation...");
    if (content_result.ok()) {
        style_mgr.apply_style_safe("BodyText", content_result.value());
    }
    
    // 5. Save document
    auto save_result = doc.save_safe();
    if (!save_result.ok()) {
        std::cerr << "Save failed: " << save_result.error().to_string() << std::endl;
        return 1;
    }
    
    std::cout << "Document created successfully!" << std::endl;
    return 0;
}
```

This guide provides comprehensive coverage of the DuckX style system. For additional examples and detailed API documentation, refer to the sample files in the `samples/` directory and the unit tests in the `test/` directory.