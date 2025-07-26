# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DuckX-PLusPlus is a C++ library for creating, reading, and writing Microsoft Word DOCX files. It features a modern hybrid error handling system using both traditional exceptions and a `Result<T>` pattern for enhanced reliability and user experience.

## Build Commands

### Windows (Using Visual Studio)

Create a temporary batch file in the `temp/` directory to build and test:

```batch
@echo off
REM Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
if %errorlevel% neq 0 (
    echo Failed to setup Visual Studio environment
    exit /b 1
)

echo Building project...
C:\Users\liuye\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe --build U:\Users\liuye\CLionProjects\duckx-custom\cmake-build-debug --target run_gtests -j 30

if %errorlevel% neq 0 (
    echo Build failed
    exit /b 1
)

echo Running tests...
cd /d U:\Users\liuye\CLionProjects\duckx-custom\cmake-build-debug\test
run_gtests.exe --gtest_brief=1
```

### Linux/macOS (Standard CMake)

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

# Run individual test
./test/test_basic
```

## Core Architecture

### Error Handling Strategy
The project implements a **hybrid error handling approach** documented in `docs/ERROR_HANDLING_STRATEGY.md`:

- **Result<T> API** (modern, recommended): Methods suffixed with `_safe()` return `Result<T>` for predictable business logic errors
- **Exception API** (legacy): Traditional methods for backward compatibility and system-level errors

```cpp
// Modern approach (preferred for new code)
auto doc_result = Document::create_safe("file.docx");
if (!doc_result.ok()) {
    handle_error(doc_result.error());
}

// Legacy approach (maintained for compatibility)
try {
    auto doc = Document::create("file.docx");  // may throw
} catch (const std::exception& e) {
    handle_exception(e);
}
```

### Key Components

1. **Document Management**
   - `Document`: Main document class with dual APIs (exception/Result<T>)
   - `DocxFile`: Low-level DOCX file operations
   - `Body`: Document body container

2. **Element Hierarchy**
   - `BaseElement`: Base class for all document elements
   - `Paragraph`, `Run`: Text content and formatting
   - `Table`, `TableRow`, `TableCell`: Table structures with extensive formatting APIs

3. **Manager Classes**
   - `MediaManager`: Image and media file handling
   - `HeaderFooterManager`: Header/footer management with type support
   - `HyperlinkManager`: Hyperlink processing and relationship management

### Error System Architecture

The error system (`Error.hpp`) provides:
- **Error Categories**: FILE_IO, XML_PARSING, DOCX_FORMAT, VALIDATION, etc.
- **Rich Context**: File location, function name, line number, additional info
- **Error Chaining**: Support for causal error relationships
- **Monadic Operations**: `and_then()`, `or_else()` for functional error handling

## API Design Patterns

### Dual API Pattern
Most operations provide both exception-based and Result<T> versions:

```cpp
// Exception-based (legacy)
void save() const;
Table& add_table(int rows, int cols);

// Result-based (modern)
Result<void> save_safe() const;
Result<Table&> add_table_safe(int rows, int cols);
```

### Fluent Interface for Table Operations
Table formatting supports method chaining with error propagation:

```cpp
auto result = table.set_width_safe(400.0)
    .and_then([](Table& t) { return t.set_alignment_safe("center"); })
    .and_then([](Table& t) { return t.set_border_style_safe("single"); });
```

## Dependencies

- **Abseil-cpp**: String utilities, status types, containers (bundled)
- **PugiXML**: XML parsing and manipulation (bundled)
- **ZIP Library**: DOCX compression/decompression (bundled)
- **STB Image**: Image processing support (bundled)
- **GoogleTest**: Testing framework (bundled, test-only)

All dependencies are bundled in `thirdparty/` and managed through CMake subprojects.

## Testing Architecture

- **Framework**: GoogleTest with unified test runner (`run_gtests`)
- **Test Structure**: Individual test files per component (`test_document.cpp`, `test_table_formatting.cpp`, etc.)
- **Resource Management**: Test resources automatically copied to build directory
- **Modern API Testing**: Comprehensive `test_result_api.cpp` for Result<T> API validation

## Code Conventions

### Error Handling
- Use `_safe()` suffix for Result<T> returning methods
- Provide both APIs during transition period
- Use `DUCKX_ERROR_CONTEXT()` macro for error context
- Chain errors using `caused_by()` for complex scenarios

### Naming Conventions
- PascalCase for classes (`Document`, `TableCell`)
- snake_case for methods (`set_width_safe`, `get_text`)
- Private members prefixed with `m_` (`m_file`, `m_body`)

### Memory Management
- Use RAII and smart pointers (`std::unique_ptr`)
- Move semantics for expensive operations
- Exception-safe resource handling

## Sample Code Patterns

Check `samples/` directory for complete examples:
- `sample1.cpp`: Basic document reading
- `sample10.cpp`: Table creation and formatting
- `sample15.cpp`: Advanced table formatting with Result<T> API
- `sample_comprehensive_test.cpp`: Full workflow demonstration

## CMake Options

- `BUILD_SAMPLES=ON`: Build example programs
- `BUILD_TESTING=ON`: Enable test building (default)
- `BUILD_SHARED_LIBS=OFF`: Build static library (default)
- `DUCKX_USE_SYSTEM_ABSL=OFF`: Use bundled Abseil (default)
- `DUCKX_ENABLE_ABSL=ON`: Enable Abseil integration (default)

## C++ Template Gotchas and Solutions

### 1. Reference Types in Template Containers
**Problem**: C++ doesn't allow reference types in certain template containers (union, optional, etc.)
```cpp
// ❌ WRONG: Will cause compilation errors
Result<Table&> // Error: reference type in template
Result<TableRow&> // Error: cannot use reference in union/optional

// ✅ CORRECT: Use pointer or value types
Result<Table*> // OK: pointer type
Result<void> // OK: void for operations without return value
```

### 2. Iterator Dereferencing
**Problem**: Iterators are not elements, they point to elements
```cpp
// ❌ WRONG: Using iterator as element
auto& row = table.rows().begin();
row.set_height_safe(10); // Error: iterator has no set_height_safe

// ✅ CORRECT: Dereference iterator first
auto& row = *table.rows().begin();
row.set_height_safe(10); // OK: row is TableRow&
```

### 3. Optional Value Access
**Problem**: `absl::optional<T>` cannot be used directly as `T`
```cpp
// ❌ WRONG: Direct access to optional content
error.context().to_string() // Error: optional has no to_string

// ✅ CORRECT: Check and access value
if (error.context().has_value()) {
    error.context().value().to_string() // OK
}
```

### 4. Monadic Operations Type Matching
**Problem**: `and_then` expects consistent return types
```cpp
// ❌ WRONG: Type mismatch in chain
body.add_paragraph_safe("test") // Returns Result<Paragraph>
    .and_then([](Paragraph& p) {
        return body.add_table_safe(2, 2); // Returns Result<Table> - TYPE MISMATCH!
    });

// ✅ CORRECT: Use sequential operations for different types
auto para_result = body.add_paragraph_safe("test");
if (para_result.ok()) {
    auto table_result = body.add_table_safe(2, 2);
}
```

### 5. GoogleTest Macro Parameter Order
**Problem**: EXPECT_EQ parameter order matters for some types
```cpp
// ❌ POTENTIALLY PROBLEMATIC: 
EXPECT_EQ(actual.value(), expected); // May cause issues

// ✅ SAFER: Expected value first
EXPECT_EQ(expected, actual.value()); // More reliable
```

## Build System Notes

### Windows Environment
- Use proper batch file syntax with delayed expansion (`setlocal enabledelayedexpansion`)
- Use `!errorlevel!` instead of `%errorlevel%` in loops
- Always use `/d` with `cd` command for drive changes
- Generator is typically Visual Studio, not Ninja (check cmake-build-debug structure)