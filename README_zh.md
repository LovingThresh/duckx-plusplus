<p align="center"><img src="img/logo.png" width="380"></p>

# DuckX-PLusPlus

一个现代化的 C++ 库，用于创建、读取和写入 Microsoft Word DOCX 文件，具有增强的可靠性和性能。

[![CI/CD Pipeline](https://github.com/LovingThresh/duckx-custom/actions/workflows/ci.yml/badge.svg)](https://github.com/LovingThresh/duckx-custom/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-14%2B-blue.svg)]()

## 🌟 关于 DuckX-PLusPlus

**DuckX-PLusPlus** 是基于原始 DuckX 库的增强版本。原始 DuckX 库在功能上存在一些限制，因此我们对其进行了全面的"PlusPlus"升级，添加了现代化的错误处理、增强的表格格式化、更好的性能以及更直观的 API 设计，让文档处理变得更加强大和易用。

### 相比原版 DuckX 的主要改进：
- ✨ **双重 API 设计**：同时支持传统异常和现代 `Result<T>` 模式
- 🛡️ **增强的错误处理**：详细的错误上下文和链式错误信息
- 🎯 **高级表格格式化**：扩展的表格样式和布局选项
- 🔗 **函数式编程支持**：使用 `and_then()` 和 `or_else()` 的单子操作
- 💾 **现代内存管理**：RAII 原则和智能指针的广泛使用
- ⚡ **性能优化**：更快的文档处理和更低的内存占用

## 🚀 核心特性

### 现代化错误处理
- **混合 API 设计**：同时支持传统异常和现代 `Result<T>` 模式
- **丰富的错误上下文**：包含文件位置和上下文的详细错误信息
- **单子操作**：使用 `and_then()` 和 `or_else()` 进行函数式错误处理

### 全面的文档支持
- **完整 DOCX 支持**：读取、写入和编辑 Microsoft Word 文档
- **高级表格格式化**：广泛的表格样式和布局选项
- **媒体管理**：图像插入和处理
- **页眉页脚支持**：完整的页眉和页脚管理
- **超链接处理**：完整的超链接关系管理

### 开发者友好的 API
- **流畅接口**：方法链式调用，直观的文档构建
- **内存安全**：RAII 原则和智能指针使用
- **异常安全**：保证资源清理
- **跨平台**：支持 Windows、Linux 和 macOS

## 📋 快速开始

### 基础文档读取（异常 API）
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
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### 现代 Result<T> API（推荐）
```cpp
#include <duckx.hpp>
#include <iostream>

int main() {
    auto doc_result = duckx::Document::create_safe("new_document.docx");
    if (!doc_result.ok()) {
        std::cerr << "创建文档失败: " 
                  << doc_result.error().to_string() << std::endl;
        return 1;
    }
    
    auto doc = std::move(doc_result.value());
    auto& body = doc.body();
    
    // 添加内容并处理错误
    auto para_result = body.add_paragraph_safe("你好，世界！");
    if (para_result.ok()) {
        para_result.value().set_alignment(duckx::Alignment::CENTER);
    }
    
    // 保存并检查错误
    auto save_result = doc.save_safe();
    if (!save_result.ok()) {
        std::cerr << "保存失败: " << save_result.error().to_string() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 高级表格格式化
```cpp
auto table_result = body.add_table_safe(3, 4);
if (table_result.ok()) {
    auto& table = table_result.value();
    
    // 流畅接口与错误传播
    auto format_result = table.set_width_safe(400.0)
        .and_then([](auto& t) { return t.set_alignment_safe("center"); })
        .and_then([](auto& t) { return t.set_border_style_safe("single"); });
        
    if (!format_result.ok()) {
        std::cerr << "表格格式化失败: " 
                  << format_result.error().to_string() << std::endl;
    }
}
```

## 🔧 构建项目

### Windows (Visual Studio)

在 `temp/` 目录中创建批处理文件：

```batch
@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
if %errorlevel% neq 0 (
    echo 设置 Visual Studio 环境失败
    exit /b 1
)

echo 构建项目...
cmake --build cmake-build-debug --target run_gtests -j 30

echo 运行测试...
cd cmake-build-debug\test
run_gtests.exe --gtest_brief=1
```

### Linux/macOS

```bash
# 标准构建
mkdir build && cd build
cmake ..
cmake --build .

# 开发构建（包含示例和测试）
cmake -DBUILD_SAMPLES=ON -DBUILD_TESTING=ON ..
cmake --build .

# 运行测试
ctest
# 或运行统一的测试可执行文件
./test/run_gtests
```

### CMake 选项

- `BUILD_SAMPLES=ON`: 构建示例程序
- `BUILD_TESTING=ON`: 启用测试构建（默认）
- `BUILD_SHARED_LIBS=OFF`: 构建静态库（默认）
- `DUCKX_USE_SYSTEM_ABSL=OFF`: 使用捆绑的 Abseil（默认）
- `DUCKX_ENABLE_ABSL=ON`: 启用 Abseil 集成（默认）

## 📚 API 设计模式

### 双重 API 支持
DuckX-PLusPlus 提供传统异常和现代 Result<T> 两种 API：

```cpp
// 基于异常（传统兼容性）
void save() const;
Table& add_table(int rows, int cols);

// 基于 Result（现代，推荐）
Result<void> save_safe() const;
Result<Table&> add_table_safe(int rows, int cols);
```

### 错误处理分类

错误系统提供全面的错误分类：
- `FILE_IO`: 文件系统操作
- `XML_PARSING`: 文档结构解析
- `DOCX_FORMAT`: DOCX 规范合规性
- `VALIDATION`: 输入验证错误
- `RESOURCE`: 资源管理问题

## 🏗️ 架构设计

### 核心组件

1. **文档管理**
   - `Document`: 具有双重 API 的主文档类
   - `DocxFile`: 底层 DOCX 文件操作
   - `Body`: 文档主体容器

2. **元素层次结构**
   - `BaseElement`: 所有文档元素的基类
   - `Paragraph`, `Run`: 文本内容和格式化
   - `Table`, `TableRow`, `TableCell`: 高级表格结构

3. **管理器类**
   - `MediaManager`: 图像和媒体文件处理
   - `HeaderFooterManager`: 支持类型的页眉页脚
   - `HyperlinkManager`: 超链接关系管理

### 依赖项（全部捆绑）

- **Abseil-cpp**: 现代 C++ 实用工具和状态类型
- **PugiXML**: 快速 XML 解析和操作
- **ZIP Library**: DOCX 压缩/解压缩
- **STB Image**: 图像处理支持
- **GoogleTest**: 测试框架（仅测试用）

## 📖 示例

探索 `samples/` 目录中的全面示例：

- `sample1.cpp`: 基础文档读取
- `sample10.cpp`: 表格创建和格式化
- `sample15.cpp`: 使用 Result<T> API 的高级表格格式化
- `sample_comprehensive_test.cpp`: 完整工作流演示

## 🧪 测试

DuckX-PLusPlus 具有全面的测试套件：

- **框架**: 带有统一测试运行器的 GoogleTest
- **覆盖率**: 90%+ 测试覆盖率目标
- **现代 API 测试**: 完整的 Result<T> API 验证
- **资源管理**: 自动测试资源复制

运行测试：
```bash
# 运行所有测试
./test/run_gtests

# 简洁输出模式
./test/run_gtests --gtest_brief=1
```

## 🛣️ 发展路线图

详细的开发计划请参见 [ROADMAP.md](docs/ROADMAP.md)：

### 近期版本 (v0.0.6 → v0.8.0)
- **v0.1.0**: 完整的 Result<T> API 覆盖和错误处理标准化
- **v0.2.0 - v0.4.0**: 增强的表格格式化和文档属性管理
- **v0.5.0 - v0.7.0**: 性能优化和样式系统实现

### 主要里程碑
- **v0.8.0** (2025年Q3): 功能完整的测试版，包含工程工具第一阶段
- **v1.0.0** (2025年Q4): 稳定版本，包含模板系统和高级工程功能

## 🤝 贡献指南

欢迎贡献！请参考我们的编码约定：

- 为返回 Result<T> 的方法使用 `_safe()` 后缀
- 遵循 RAII 和异常安全模式
- 类使用 PascalCase，方法使用 snake_case
- 为新功能提供全面的单元测试

## 📄 许可证

本库采用 MIT 许可证。详情请参见 [LICENSE](LICENSE)。

## 🙏 致谢

DuckX-PLusPlus 基于原始 DuckX 库的基础构建，为专业文档处理需求进行了广泛的现代化和功能增强。

---

**用 ❤️ 为现代 C++ 开发而构建**