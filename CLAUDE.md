# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 基本原则 (Fundamental Principles)

**⚠️ CRITICAL: Git Command Restrictions**
- **NEVER** use git modification commands: `git add`, `git commit`, `git push`, `git merge`, `git rebase`, etc.
- **ONLY** use git query commands: `git status`, `git log`, `git diff`, `git show`, `git branch --list`
- **ALWAYS** provide commit messages as text output when requested, but NEVER execute the commit
- User must manually execute all git modification operations
- This ensures user maintains full control over repository state and commit history

## Project Overview

DuckX-PLusPlus is a C++ library for creating, reading, and writing Microsoft Word DOCX files. It features a modern hybrid error handling system using both traditional exceptions and a `Result<T>` pattern for enhanced reliability and user experience.

## Build Commands

### Windows (Multiple Compiler Support)

The project supports multiple compiler toolchains on Windows:

**MSVC (Primary - Fully Supported)**
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

**MinGW GCC (Experimental)**
```batch
REM Configure with MinGW
cmake -B cmake-build-debug-mingw -DCMAKE_BUILD_TYPE=Debug -DBUILD_SAMPLES=ON -DBUILD_TESTING=ON -G "Ninja"

REM Build
C:\Users\liuye\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe --build cmake-build-debug-mingw --target run_gtests -j 30
```

**Visual Studio Clang (Experimental)**  
```batch
REM Configure with Clang
cmake -B cmake-build-debug-visual-studio-clang -DCMAKE_BUILD_TYPE=Debug -DBUILD_SAMPLES=ON -DBUILD_TESTING=ON -T ClangCL

REM Build
C:\Users\liuye\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe --build cmake-build-debug-visual-studio-clang --target run_gtests -j 30
```

**Build Status**:
- ✅ **MSVC**: Fully working, all 190 tests pass
- ⚠️ **MinGW GCC**: May require environment-specific configuration, works in CLion IDE  
- ⚠️ **Visual Studio Clang**: May require environment-specific configuration, works in CLion IDE

**Note**: Alternative compilers (MinGW/Clang) may face compatibility issues with bundled Abseil library in certain environments. For production builds, MSVC is recommended.

### Linux/macOS/WSL (Standard CMake)

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

### WSL Environment (Recommended for Development)

**Environment Setup**:
- Uses native WSL cmake and build tools for optimal compatibility
- Build directory: `cmake-build-debug-wsl/`
- Requires sudo access for temp directory creation

**Build Commands**:
```bash
# Build tests with parallel processing
cmake --build cmake-build-debug-wsl --target run_gtests -j 8

# Build specific targets
cmake --build cmake-build-debug-wsl --target duckx
cmake --build cmake-build-debug-wsl --target sample1

# Run tests (all 290 tests pass)
cd cmake-build-debug-wsl/test && ./run_gtests --gtest_brief=1

# Run samples
cd cmake-build-debug-wsl/samples && ./sample1
```

**Permission Management**:
```bash
# Create required temp directories with proper permissions
sudo mkdir -p cmake-build-debug-wsl/test/temp
sudo chmod 777 cmake-build-debug-wsl/test/temp
```

**Advantages over Windows builds**:
- ✅ No compiler environment setup issues
- ✅ Better standard library compatibility
- ✅ Native Linux toolchain reliability

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

### C++14 Compatibility and Development Guidelines

#### C++ Standard Version Compliance
**Core Principle**: Maintain strict C++14 compatibility throughout the codebase

**Strategy for Modern C++ Features**:
1. **First**: Check if the feature exists in **Abseil-cpp** (bundled dependency)
2. **Second**: If not available in Abseil, implement C++14-compatible alternative
3. **Never**: Use C++17/20/23 features directly without compatibility layer

**Standard Library Feature Compatibility**:
```cpp
// ❌ WRONG: Using newer standard features
std::string str = "example";
if (str.starts_with("ex")) { ... }        // C++20
if (str.contains("amp")) { ... }          // C++23  
std::optional<int> opt = std::nullopt;     // C++17

// Structured bindings (C++17)
for (const auto& [key, value] : map) { ... }

// Nested namespace definition (C++17)
namespace company::project::module { ... }

// ✅ CORRECT: C++14 compatible or Abseil alternatives
#include "absl/strings/str_format.h"       // Use Abseil string utilities
#include "absl/types/optional.h"           // Use absl::optional instead of std::optional

if (!str.empty() && str[0] == 'e') { ... }                    // Manual prefix check
if (str.find("amp") != std::string::npos) { ... }             // Manual substring search
absl::optional<int> opt = absl::nullopt;                      // Abseil optional

// Traditional pair access (C++14)
for (const auto& pair : map) {
    const auto& key = pair.first;
    const auto& value = pair.second;
    // ...
}

// Traditional nested namespace (C++14)
namespace company {
namespace project {
namespace module {
    // ...
} // namespace module
} // namespace project
} // namespace company
```

#### Error Handling Type System Compliance
**Core Problem**: Template type systems require explicit construction for type safety

**Error Return Pattern**:
```cpp
// ❌ WRONG: Implicit conversion may fail with explicit constructors
return errors::validation_failed("field", "reason");

// ✅ CORRECT: Always use explicit Result<T> construction
return Result<void>(errors::validation_failed("field", "reason"));
return Result<SomeType*>(errors::element_not_found("element"));
```

**Error Factory Function Dependencies**:
- **Before using**: Verify error factory function exists in `Error.hpp`
- **Pattern**: All error functions must be defined in `namespace duckx::errors`
- **Consistency**: Match ErrorCategory and ErrorCode enums

#### Dependency Resolution Strategy
**Abseil-first Approach**:
1. **String Operations**: Use `absl/strings/` utilities before manual implementation
2. **Container Types**: Use `absl/container/` flat_hash_map, flat_hash_set, etc.
3. **Type Utilities**: Use `absl/types/` optional, variant, span
4. **Status Types**: Use `absl/status/` for compatibility layers

**Manual Implementation Guidelines**:
- Only implement when Abseil equivalent doesn't exist
- Maintain C++14 compatibility
- Document why manual implementation was chosen
- Consider future migration path to standard library

#### Development Workflow
**Pre-implementation Checklist**:
1. ✅ Verify C++14 compatibility of all language features used
2. ✅ Check Abseil documentation for equivalent functionality  
3. ✅ Ensure all error factory functions are declared before use
4. ✅ Use explicit `Result<T>` construction for error returns
5. ✅ Test with MSVC (primary target compiler)

## Git-Related Information

- **Git Flow**: This project follows a modified Git Flow strategy
- **Branching Strategy**:
  - `main`: Stable production releases
  - `develop`: Ongoing development branch
  - `feature/`: Individual feature development branches
  - `bugfix/`: Bug resolution branches
  - `hotfix/`: Urgent production fixes
- **Commit Message Guidelines**:
  - Use descriptive, concise commit messages
  - Prefix with type: `feat:`, `fix:`, `docs:`, `refactor:`, `test:`, `chore:`
- **Recommended Git Commands**:
  - `git flow init`: Initialize Git Flow in repository
  - `git flow feature start <name>`: Start a new feature branch
  - `git flow feature finish <name>`: Complete a feature branch
  - `git flow release start <version>`: Create a new release branch
- **Common Git Workflows**:
  - Always pull before starting work
  - Use `git rebase` instead of `git merge` for cleaner history
  - Use meaningful branch names
  - Squash commits before merging feature branches
  - **Repository Management**:
    - Use multiple repository management techniques, carefully check code standards and branch status before committing
    - Git information should be as detailed as possible, covering key context of code changes
    - Maintain readability and professionalism in commit messages

## Code Generation and Documentation Workflows

### Language Usage Guidelines
- **When writing Markdown documentation**: Use English
- **When writing git commit messages**: Use Chinese

### Commit Message Generation for Code Comments

When generating git commit messages for code comment updates:

  1. Use concise Chinese format following project commit style
  2. Do not include bot signatures
  3. Organize with this structure:
    - Concise title line (e.g., docs(core): 完善核心代码库 Doxygen 注释体系)
    - Overview description
    - "核心改进" section listing specific changes
    - "覆盖核心模块" section describing involved modules
  4. Highlight key points:
    - File count statistics
    - Comment standards and formats
    - Adherence to project specifications
    - Core functional modules involved
  5. Only provide commit message content, do not execute actual git commits
