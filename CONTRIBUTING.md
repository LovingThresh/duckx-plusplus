# Contributing to DuckX-PLusPlus

Thank you for your interest in contributing to DuckX-PLusPlus! We welcome contributions from the community and are excited to work with you.

## 🚀 Getting Started

When contributing to this repository, please first discuss the change you wish to make via:
- Opening an issue
- Joining our discussions
- Contacting the maintainers

This helps ensure your contribution aligns with the project goals and avoids duplicate work.

## 📋 Development Setup

### Prerequisites
- **C++14 or later** (project uses C++14 standard)
- **CMake 3.10+**
- **Visual Studio 2022** (Windows) or **GCC 7+/Clang 6+** (Linux/macOS)

### Building the Project

Follow the build instructions in [README.md](README.md) or [CLAUDE.md](CLAUDE.md).

## 🔧 Code Conventions

### Modern C++ Guidelines

1. **Error Handling**
   - Use `_safe()` suffix for Result<T> returning methods
   - Provide both APIs during transition period (exception and Result<T>)
   - Use `DUCKX_ERROR_CONTEXT()` macro for error context
   - Chain errors using `caused_by()` for complex scenarios

2. **Naming Conventions**
   - **PascalCase** for classes: `Document`, `TableCell`, `MediaManager`
   - **snake_case** for methods: `set_width_safe`, `get_text`, `add_paragraph`
   - **Private members** prefixed with `m_`: `m_file`, `m_body`, `m_document`
   - **Constants** in UPPER_SNAKE_CASE: `MAX_TABLE_ROWS`

3. **Documentation Standards**
   - Follow the [Documentation Style Guide](docs/DOCUMENTATION_STYLE.md)
   - **English only** for all comments and documentation
   - Document **file purpose**, **class responsibilities**, and **key methods**
   - Add **inline comments** only for complex algorithms
   - Use **Doxygen-compatible** syntax with `@brief`, `@param`, `@return`

4. **Memory Management**
   - Use RAII principles and smart pointers (`std::unique_ptr`, `std::shared_ptr`)
   - Prefer move semantics for expensive operations
   - Ensure exception-safe resource handling
   - Avoid raw pointers except for non-owning references

5. **API Design Patterns**
   - **Dual API Support**: Provide both exception-based and Result<T> versions
   ```cpp
   // Exception-based (legacy compatibility)
   void save() const;
   Table& add_table(int rows, int cols);
   
   // Result-based (modern, recommended)
   Result<void> save_safe() const;
   Result<Table&> add_table_safe(int rows, int cols);
   ```
   - **Fluent Interface**: Support method chaining where appropriate
   - **Const Correctness**: Mark methods const when they don't modify state

### Code Quality Standards

1. **Testing Requirements**
   - **Unit tests** for all new functionality
   - **Test coverage** should be ≥ 90% for new code
   - Use GoogleTest framework
   - Include both positive and negative test cases
   - Test both exception and Result<T> APIs

2. **Documentation Standards**
   - **API documentation** for all public methods
   - **Code comments** explaining complex algorithms
   - **Examples** for new features in samples/ directory
   - **Update documentation** when changing APIs

3. **Performance Guidelines**
   - Profile performance-critical code paths
   - Minimize memory allocations in hot paths
   - Use move semantics and perfect forwarding where appropriate
   - Benchmark against existing implementations

## 📝 Pull Request Process

### Before Submitting

1. **Code Quality Checks**
   - Ensure all tests pass: `./test/run_gtests`
   - Follow the coding style guidelines above
   - Add appropriate unit tests for your changes
   - Update documentation if needed

2. **PR Guidelines**
   - **Keep PRs focused**: Handle one specific topic per PR
   - **Write clear commit messages**: Follow our commit format (see [COMMIT_MESSAGE_TEMPLATE.md](COMMIT_MESSAGE_TEMPLATE.md))
   - **Include tests**: All new features must include comprehensive tests
   - **Update samples**: Add usage examples for significant new features

### Commit Message Format

Follow our established format:
```
<type>(<scope>): <subject>

<body with bullet points>

<footer if needed>
```

**Types**: `feat`, `fix`, `chore`, `docs`, `test`, `refactor`
**Scopes**: `core`, `API`, `build`, `docs`, `tests`

### Review Process

1. **Automated Checks**: All PRs must pass automated tests
2. **Code Review**: At least one maintainer review required
3. **Documentation Review**: Ensure documentation is updated
4. **Integration Testing**: Verify changes work with existing code

## 🧪 Testing Guidelines

### Test Structure

```cpp
// Example test structure
TEST(ComponentTest, SpecificFunctionality) {
    // Arrange
    auto doc_result = Document::create_safe("test.docx");
    ASSERT_TRUE(doc_result.ok());
    
    // Act
    auto result = doc_result.value().some_operation_safe();
    
    // Assert
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(expected_value, result.value());
}
```

### Testing Both APIs

Always test both exception and Result<T> versions:

```cpp
TEST(DocumentTest, SaveBothAPIs) {
    // Test Result<T> API
    auto doc_result = Document::create_safe("test.docx");
    ASSERT_TRUE(doc_result.ok());
    auto save_result = doc_result.value().save_safe();
    EXPECT_TRUE(save_result.ok());
    
    // Test exception API
    EXPECT_NO_THROW({
        auto doc = Document::create("test2.docx");
        doc.save();
    });
}
```

## 🎯 Areas for Contribution

### High Priority
- **Result<T> API Coverage**: Complete migration of remaining methods
- **Table Formatting**: Advanced table styling features
- **Error Handling**: Enhanced error context and recovery mechanisms

### Medium Priority
- **Performance Optimization**: Memory usage and parsing speed improvements
- **Engineering Tools**: Technical document formatting utilities
- **Cross-platform Testing**: Ensure compatibility across platforms

### Future Features
- **Document Templates**: Template engine implementation
- **Document Comparison**: Diff and merge functionality
- **Advanced Formatting**: Complex document styling features

## 📚 Resources

- [Project Roadmap](docs/ROADMAP.md) - Development plans and priorities
- [Error Handling Strategy](docs/ERROR_HANDLING_STRATEGY.md) - Error handling guidelines
- [API Examples](docs/ERROR_HANDLING_EXAMPLES.hpp) - Usage examples
- [Sample Code](samples/) - Complete working examples

## 🤝 Code of Conduct

### Our Commitment

We are committed to fostering an open and welcoming environment where everyone can contribute effectively, regardless of experience level, background, or perspective.

### Standards

**Positive behaviors include:**
- Using welcoming and inclusive language
- Being respectful of differing viewpoints and experiences
- Gracefully accepting constructive criticism
- Focusing on what is best for the community
- Showing empathy towards other community members

**Unacceptable behaviors include:**
- Harassment, trolling, or insulting comments
- Personal or political attacks
- Publishing others' private information without permission
- Any conduct that could reasonably be considered inappropriate

### Enforcement

Instances of abusive, harassing, or otherwise unacceptable behavior may be reported by contacting the project maintainers. All complaints will be reviewed and investigated promptly and fairly.

## 📄 License

By contributing to DuckX-PLusPlus, you agree that your contributions will be licensed under the same MIT License that covers the project.

---

**Thank you for contributing to DuckX-PLusPlus! 🚀**