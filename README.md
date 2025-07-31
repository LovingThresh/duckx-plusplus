<p align="center"><img src="img/logo.png" width="380"></p>

# DuckX-PLusPlus

A modern C++ library for creating, reading, and writing Microsoft Word DOCX files with enhanced reliability and performance.

## 🌟 About DuckX-PLusPlus

**DuckX-PLusPlus** is an enhanced version built upon the foundation of the original DuckX library. While the original DuckX provided basic DOCX functionality, it had several limitations in terms of error handling, API design, and advanced features. This "PlusPlus" version addresses these limitations with comprehensive modernization:

### Key Improvements Over Original DuckX:
- ✨ **Dual API Design**: Both traditional exceptions and modern `Result<T>` pattern support
- 🛡️ **Enhanced Error Handling**: Rich error context with detailed information and error chaining
- 🎯 **Advanced Table Formatting**: Extensive table styling and layout options beyond basic functionality
- 🔗 **Functional Programming Support**: Monadic operations with `and_then()` and `or_else()`
- 💾 **Modern Memory Management**: Extensive use of RAII principles and smart pointers
- ⚡ **Performance Optimizations**: Faster document processing and lower memory footprint

**[中文版 README](README_zh.md)** | **[English README](README.md)**

[![CI/CD Pipeline](https://github.com/LovingThresh/duckx-custom/actions/workflows/ci.yml/badge.svg)](https://github.com/LovingThresh/duckx-custom/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-14%2B-blue.svg)]()

---

## 📋 快速导航

| 📚 [**示例程序**](@ref samples_page) | 🧪 [**测试程序**](@ref tests_page) | 🔧 [**API 文档**](@ref annotated) | 🗂️ [**导航总览**](@ref navigation_page) |
|:---:|:---:|:---:|:---:|
| 27个实际应用示例 | 完整测试套件说明 | 类和函数参考 | 快速访问指南 |

---

## 🚀 Key Features

### Modern Error Handling
- **Hybrid API Design**: Both traditional exceptions and modern `Result<T>` pattern
- **Rich Error Context**: Detailed error information with file location and context
- **Monadic Operations**: Functional error handling with `and_then()` and `or_else()`

### Comprehensive Document Support
- **Full DOCX Support**: Read, write, and edit Microsoft Word documents
- **Advanced Table Formatting**: Extensive table styling and layout options
- **Style System**: Complete style management with built-in and custom styles
- **XML Style System**: User-friendly XML-based style definition and management
- **Media Management**: Image insertion and handling
- **Header/Footer Support**: Complete header and footer management
- **Hyperlink Processing**: Full hyperlink relationship management
- **Document Outline & TOC**: Automatic outline generation and field-based table of contents
- **Advanced Navigation**: Bookmark-based document navigation with Word-compatible TOC fields

### Developer-Friendly API
- **Fluent Interface**: Method chaining for intuitive document construction
- **Memory Safe**: RAII principles and smart pointer usage
- **Exception Safe**: Guaranteed resource cleanup
- **Cross-Platform**: Windows, Linux, and macOS support

## 📋 Quick Start

### Basic Document Reading (Exception API)
```cpp
#include <duckx.hpp>
#include <iostream>

int main() {
    try {
        auto doc = duckx::Document::open("file.docx");
        
        for (auto& paragraph : doc.body().paragraphs()) {
            for (auto& run : paragraph.runs()) {
                std::cout << run.get_text();
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Modern Result<T> API (Recommended)
```cpp
#include <duckx.hpp>
#include <iostream>

int main() {
    auto doc_result = duckx::Document::create_safe("new_document.docx");
    if (!doc_result.ok()) {
        std::cerr << "Failed to create document: " 
                  << doc_result.error().to_string() << std::endl;
        return 1;
    }
    
    auto doc = std::move(doc_result.value());
    auto& body = doc.body();
    
    // Add content with error handling
    auto para_result = body.add_paragraph_safe("Hello, World!");
    if (para_result.ok()) {
        para_result.value().set_alignment(duckx::Alignment::CENTER);
    }
    
    // Save with error checking
    auto save_result = doc.save_safe();
    if (!save_result.ok()) {
        std::cerr << "Save failed: " << save_result.error().to_string() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Advanced Table Formatting
```cpp
auto table_result = body.add_table_safe(3, 4);
if (table_result.ok()) {
    auto& table = table_result.value();
    
    // Fluent interface with error propagation
    auto format_result = table.set_width_safe(400.0)
        .and_then([](auto& t) { return t.set_alignment_safe("center"); })
        .and_then([](auto& t) { return t.set_border_style_safe("single"); });
        
    if (!format_result.ok()) {
        std::cerr << "Table formatting failed: " 
                  << format_result.error().to_string() << std::endl;
    }
}
```

### Style System Integration
```cpp
// Create and apply custom styles
auto& styles = doc.styles();

// Create a custom paragraph style
auto heading_result = styles.create_mixed_style_safe("Custom Heading");
if (heading_result.ok()) {
    auto* style = heading_result.value();
    
    // Configure paragraph properties
    duckx::ParagraphStyleProperties para_props;
    para_props.alignment = duckx::Alignment::CENTER;
    para_props.space_after_pts = 12.0;
    
    // Configure character properties
    duckx::CharacterStyleProperties char_props;
    char_props.font_name = "Arial";
    char_props.font_size_pts = 18.0;
    char_props.font_color_hex = "2F5496";
    
    style->set_paragraph_properties_safe(para_props);
    style->set_character_properties_safe(char_props);
}

// Apply style to document elements
auto para_result = body.add_paragraph_safe("Styled Heading");
if (para_result.ok()) {
    auto apply_result = para_result.value().apply_style_safe(styles, "Custom Heading");
    if (!apply_result.ok()) {
        std::cerr << "Style application failed: " << apply_result.error().to_string() << std::endl;
    }
}

// Use built-in styles
auto normal_para = body.add_paragraph_safe("Normal text");
if (normal_para.ok()) {
    normal_para.value().apply_style_safe(styles, "Normal");
}
```

### XML Style System (New!)
```cpp
#include "XmlStyleParser.hpp"

// Load styles from XML file
XmlStyleParser parser;
auto styles_result = parser.load_styles_from_file_safe("document_styles.xml");
if (styles_result.ok()) {
    auto& styles = styles_result.value();
    std::cout << "Loaded " << styles.size() << " styles from XML" << std::endl;
}

// Load style sets for themed documents
auto style_sets_result = parser.load_style_sets_from_file_safe("document_styles.xml");
if (style_sets_result.ok()) {
    auto& style_sets = style_sets_result.value();
    // Apply coordinated style collections
}
```

### Document Outline & Table of Contents (New!)
```cpp
#include "OutlineManager.hpp"

// Create document with heading structure
auto doc_result = duckx::Document::create_safe("document.docx");
if (doc_result.ok()) {
    auto& doc = doc_result.value();
    auto& body = doc.body();
    auto& styles = doc.styles();
    
    // Initialize OutlineManager
    OutlineManager outline(&doc, &styles);
    
    // Add structured content with heading styles
    auto title = body.add_paragraph_safe("Document Title");
    title.value().apply_style_safe(styles, "Title");
    
    auto h1 = body.add_paragraph_safe("1. Introduction");
    h1.value().apply_style_safe(styles, "Heading 1");
    
    auto h2 = body.add_paragraph_safe("1.1 Purpose");
    h2.value().apply_style_safe(styles, "Heading 2");
    
    // Add placeholder for TOC at beginning
    body.add_paragraph_safe("[Table of Contents will be inserted here]");
    
    // Generate field-based TOC after content creation
    TocOptions toc_options;
    toc_options.toc_title = "Table of Contents";
    toc_options.max_level = 3;
    toc_options.show_page_numbers = true;
    toc_options.use_hyperlinks = true;
    
    // Create real Word-compatible TOC with bookmarks
    auto toc_result = outline.create_field_toc_at_placeholder_safe(
        "[Table of Contents will be inserted here]", toc_options);
    
    if (toc_result.ok()) {
        std::cout << "✓ Created updateable TOC with hyperlinks!" << std::endl;
        // TOC will be automatically updated in Word
    }
    
    // Generate document outline
    auto outline_result = outline.generate_outline_safe();
    if (outline_result.ok()) {
        for (const auto& entry : outline_result.value()) {
            std::cout << "Level " << entry.level << ": " << entry.text << std::endl;
        }
    }
}
```

## 🔧 Building the Project

### Windows (Visual Studio)

Create a batch file in the `temp/` directory:

```batch
@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
if %errorlevel% neq 0 (
    echo Failed to setup Visual Studio environment
    exit /b 1
)

echo Building project...
cmake --build cmake-build-debug --target run_gtests -j 30

echo Running tests...
cd cmake-build-debug\test
run_gtests.exe --gtest_brief=1
```

### Linux/macOS

```bash
# Standard build
mkdir build && cd build
cmake ..
cmake --build .

# Development build with samples and tests
cmake -DBUILD_SAMPLES=ON -DBUILD_TESTING=ON ..
cmake --build .

# Run tests
ctest
# Or run the unified test executable
./test/run_gtests
```

### CMake Options

- `BUILD_SAMPLES=ON`: Build example programs
- `BUILD_TESTING=ON`: Enable test building (default)
- `BUILD_SHARED_LIBS=OFF`: Build static library (default)
- `DUCKX_USE_SYSTEM_ABSL=OFF`: Use bundled Abseil (default)
- `DUCKX_ENABLE_ABSL=ON`: Enable Abseil integration (default)

## 📚 API Design Patterns

### Dual API Support
DuckX-PLusPlus provides both traditional exception-based and modern Result<T> APIs:

```cpp
// Exception-based (legacy compatibility)
void save() const;
Table& add_table(int rows, int cols);

// Result-based (modern, recommended)
Result<void> save_safe() const;
Result<Table&> add_table_safe(int rows, int cols);
```

### Error Handling Categories

The error system provides comprehensive error categorization:
- `FILE_IO`: File system operations
- `XML_PARSING`: Document structure parsing
- `DOCX_FORMAT`: DOCX specification compliance
- `VALIDATION`: Input validation errors
- `RESOURCE`: Resource management issues

## 🏗️ Architecture

### Core Components

1. **Document Management**
   - `Document`: Main document class with dual APIs
   - `DocxFile`: Low-level DOCX file operations
   - `Body`: Document body container

2. **Element Hierarchy**
   - `BaseElement`: Base class for all document elements
   - `Paragraph`, `Run`: Text content and formatting
   - `Table`, `TableRow`, `TableCell`: Advanced table structures

3. **Manager Classes**
   - `StyleManager`: Style creation, management, and application
   - `MediaManager`: Image and media file handling
   - `HeaderFooterManager`: Header/footer with type support
   - `HyperlinkManager`: Hyperlink relationship management

### Dependencies (All Bundled)

- **Abseil-cpp**: Modern C++ utilities and status types
- **PugiXML**: Fast XML parsing and manipulation
- **ZIP Library**: DOCX compression/decompression
- **STB Image**: Image processing support
- **GoogleTest**: Testing framework (test-only)

## 📖 Examples

Explore the comprehensive examples in the `samples/` directory:

- `sample1.cpp`: Basic document reading
- `sample10.cpp`: Table creation and formatting
- `sample15.cpp`: Advanced table formatting with Result<T> API
- `sample20_complete_style_system.cpp`: Complete style system demonstration
- `sample21_style_priority_test.cpp`: Style vs direct formatting priority
- `sample25_xml_style_parser.cpp`: XML-based style definition system
- `sample_comprehensive_test.cpp`: Full workflow demonstration

## 🧪 Testing

DuckX-PLusPlus features a comprehensive testing suite:

- **Framework**: GoogleTest with unified test runner
- **Coverage**: 90%+ test coverage target with standardized Doxygen documentation
- **Modern API Testing**: Complete Result<T> API validation across 47+ test and sample files
- **Resource Management**: Automatic test resource copying
- **Documentation Standards**: Professional Doxygen comment format (2025.07) across entire codebase

Run tests with:
```bash
# Run all tests
./test/run_gtests

# Run with brief output
./test/run_gtests --gtest_brief=1
```

## 🛣️ Roadmap

See [ROADMAP.md](docs/ROADMAP.md) for detailed development plans:

### Near-term Releases (v0.0.6 → v0.8.0)
- **v0.1.0**: Complete Result<T> API coverage and error handling standardization
- **v0.2.0 - v0.4.0**: Enhanced table formatting and document properties management
- **v0.5.0 - v0.6.0**: Performance optimization and style system implementation ✅ **Completed**
- **v0.6.5**: XML Style System implementation ✅ **Completed**
- **v0.7.0**: Testing and documentation enhancement

### Major Milestones
- **v0.6.0** (Q3 2025): ✅ **Completed** - Style system implementation with comprehensive comment standardization
- **v0.8.0** (Q3 2025): Feature-complete Beta version with engineering tools Phase 1
- **v1.0.0** (Q4 2025): Stable release with template system and advanced engineering features

## 🤝 Contributing

Contributions are welcome! Please see our coding conventions:

- Use `_safe()` suffix for Result<T> returning methods
- Follow RAII and exception-safe patterns
- PascalCase for classes, snake_case for methods
- Comprehensive unit testing for new features

## 📄 License

This library is available under the MIT License. See [LICENSE](LICENSE) for details.

## 🙏 Acknowledgments

DuckX-PLusPlus is built upon the foundation of the original [DuckX library](https://github.com/amiremohamadi/DuckX), with extensive modernization and feature enhancements for professional document processing needs. We extend our gratitude to the original DuckX contributors for providing the solid foundation that made this enhanced version possible.

## 🤖 AI-Assisted Development

This project has been significantly enhanced with the assistance of **Claude Code**, Anthropic's AI-powered development tool. The comprehensive comment standardization, documentation improvements, and code quality enhancements across 47+ files were accomplished through AI-human collaboration.

**⚠️ Important Note**: As with any AI-generated or AI-assisted code, please:
- **Review carefully** before using in production environments
- **Test thoroughly** to ensure functionality meets your requirements  
- **Validate security** aspects and error handling patterns
- **Report issues** if you encounter any problems or inconsistencies

We welcome questions, feedback, and contributions from the community to continuously improve this library.

---

**Built with ❤️ for modern C++ development**