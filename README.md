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

## 🚀 Key Features

### Modern Error Handling
- **Hybrid API Design**: Both traditional exceptions and modern `Result<T>` pattern
- **Rich Error Context**: Detailed error information with file location and context
- **Monadic Operations**: Functional error handling with `and_then()` and `or_else()`

### Comprehensive Document Support
- **Full DOCX Support**: Read, write, and edit Microsoft Word documents
- **Advanced Table Formatting**: Extensive table styling and layout options
- **Media Management**: Image insertion and handling
- **Header/Footer Support**: Complete header and footer management
- **Hyperlink Processing**: Full hyperlink relationship management

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
- `sample_comprehensive_test.cpp`: Full workflow demonstration

## 🧪 Testing

DuckX-PLusPlus features a comprehensive testing suite:

- **Framework**: GoogleTest with unified test runner
- **Coverage**: 90%+ test coverage target
- **Modern API Testing**: Complete Result<T> API validation
- **Resource Management**: Automatic test resource copying

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
- **v0.5.0 - v0.7.0**: Performance optimization and style system implementation

### Major Milestones
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

---

**Built with ❤️ for modern C++ development**