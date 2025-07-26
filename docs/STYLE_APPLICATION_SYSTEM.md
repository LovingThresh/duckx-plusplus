# Style Application System

## 概述

DuckX-PLusPlus 的样式应用系统提供了完整的文档样式管理和应用功能，支持将 StyleManager 创建的样式应用到实际的文档元素上。

## 核心功能

### 1. 样式应用 API

#### Paragraph 样式应用
```cpp
// 应用段落或混合样式
Result<void> apply_style_safe(const StyleManager& style_manager, const std::string& style_name);

// 获取当前样式
Result<std::string> get_style_safe() const;

// 移除样式
Result<void> remove_style_safe();
```

#### Run 字符样式应用
```cpp
// 应用字符或混合样式
Result<void> apply_style_safe(const StyleManager& style_manager, const std::string& style_name);

// 获取当前样式
Result<std::string> get_style_safe() const;

// 移除样式
Result<void> remove_style_safe();
```

#### Table 表格样式应用
```cpp
// 应用表格或混合样式
Result<void> apply_style_safe(const StyleManager& style_manager, const std::string& style_name);

// 获取当前样式
Result<std::string> get_style_safe() const;

// 移除样式
Result<void> remove_style_safe();
```

### 2. Document 集成

Document 类现在包含内置的 StyleManager：

```cpp
class Document {
public:
    StyleManager& styles() const;
    // ... 其他方法
};
```

使用示例：
```cpp
auto doc = Document::create_safe("output.docx");
auto& style_manager = doc.value().styles();
auto& body = doc.value().body();

// 加载内置样式
style_manager.load_all_built_in_styles_safe();

// 添加段落并应用样式
auto para = body.add_paragraph_safe("Heading Text");
para.value()->apply_style_safe(style_manager, "Heading 1");
```

## 样式兼容性

### 样式类型与元素兼容性

| 样式类型 | Paragraph | Run | Table |
|---------|-----------|-----|-------|
| PARAGRAPH | ✅ | ❌ | ❌ |
| CHARACTER | ❌ | ✅ | ❌ |
| TABLE | ❌ | ❌ | ✅ |
| MIXED | ✅ | ✅ | ✅ |

### 错误处理

系统会自动验证样式兼容性：
- 尝试将字符样式应用到段落会返回错误
- 尝试将段落样式应用到文本运行会返回错误
- 混合样式可以应用到任何元素

## XML 输出

样式应用会在 DOCX XML 中生成正确的样式引用：

### 段落样式
```xml
<w:p>
    <w:pPr>
        <w:pStyle w:val="Heading 1"/>
    </w:pPr>
    <w:r>
        <w:t>Heading Text</w:t>
    </w:r>
</w:p>
```

### 字符样式
```xml
<w:r>
    <w:rPr>
        <w:rStyle w:val="Code"/>
    </w:rPr>
    <w:t>Code text</w:t>
</w:r>
```

### 表格样式
```xml
<w:tbl>
    <w:tblPr>
        <w:tblStyle w:val="Professional Table"/>
    </w:tblPr>
    <!-- 表格内容 -->
</w:tbl>
```

## 实现架构

### 核心组件

1. **BaseElement.hpp/cpp**: 为 Paragraph、Run、Table 类添加样式应用方法
2. **Document.hpp/cpp**: 集成 StyleManager 到 Document 类
3. **Error.hpp**: 添加 `xml_manipulation_failed` 错误处理函数

### 样式应用流程

1. **验证**: 检查样式是否存在且与目标元素兼容
2. **XML 操作**: 获取或创建元素的属性节点
3. **样式引用**: 添加或更新样式引用节点
4. **错误处理**: 提供详细的错误信息和上下文

## 使用示例

### 基本用法

```cpp
#include "duckx.hpp"

int main() {
    auto doc = Document::create_safe("styled_document.docx");
    auto& style_manager = doc.value().styles();
    auto& body = doc.value().body();
    
    // 加载内置样式
    style_manager.load_all_built_in_styles_safe();
    
    // 创建自定义样式
    auto custom_style = style_manager.create_mixed_style_safe("Custom Title");
    // ... 配置样式属性
    
    // 应用样式到文档元素
    auto title = body.add_paragraph_safe("Document Title");
    title.value()->apply_style_safe(style_manager, "Custom Title");
    
    auto paragraph = body.add_paragraph_safe("Normal text");
    paragraph.value()->apply_style_safe(style_manager, "Normal");
    
    doc.value().save_safe();
    return 0;
}
```

### 高级用法 - 混合内容样式

```cpp
// 创建包含不同字符样式的段落
auto para = body.add_paragraph_safe("");
para.value()->apply_style_safe(style_manager, "Normal");

// 添加不同样式的文本运行
Run& normal_text = para.value()->add_run("Normal text ");
Run& code_text = para.value()->add_run("code example");
Run& emphasis_text = para.value()->add_run(" emphasized text");

// 应用字符样式
code_text.apply_style_safe(style_manager, "Code");
emphasis_text.apply_style_safe(style_manager, "Important Text");
```

## 测试覆盖

### 单元测试

- `test_style_application.cpp`: 全面的样式应用测试
- 包含兼容性验证、错误处理、XML 生成测试

### 示例程序

- `sample20_style_application.cpp`: 完整的样式应用演示
- `sample21_simple_style_test.cpp`: 基本功能验证
- 演示了内置样式和自定义样式的应用

## 未来扩展

### 计划功能

1. **样式继承解析**: 自动解析和应用继承链中的样式属性
2. **DOCX 样式集成**: 将 StyleManager 生成的 styles.xml 集成到 DOCX 文件结构
3. **实时样式预览**: 提供样式效果的预览功能
4. **批量样式操作**: 支持批量应用和修改样式

### 性能优化

1. **样式缓存**: 缓存常用样式的 XML 结构
2. **延迟加载**: 按需加载样式定义
3. **XML 优化**: 优化样式 XML 的生成和应用

## 兼容性

- **C++14 标准**: 严格遵循 C++14 兼容性
- **异常安全**: 所有操作都是异常安全的
- **向后兼容**: 保持与现有 API 的兼容性

## 错误代码

样式应用系统使用以下错误类别：

- `ErrorCategory::STYLE_SYSTEM`: 样式系统相关错误
- `ErrorCategory::XML_PARSING`: XML 操作错误
- `ErrorCode::STYLE_PROPERTY_INVALID`: 样式属性不兼容
- `ErrorCode::XML_MANIPULATION_FAILED`: XML 操作失败

每个错误都包含详细的上下文信息，便于调试和问题定位。