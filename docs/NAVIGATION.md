# 快速导航 {#navigation_page}

## 📚 主要功能模块

| [🏠 **主页**](index.html) | [📋 **示例程序**](samples_page.html) | [🧪 **测试程序**](tests_page.html) | [📖 **API 文档**](annotated.html) | [📁 **文件列表**](files.html) | [💡 **详细示例**](examples.html) |
|:---:|:---:|:---:|:---:|:---:|:---:|
| 项目介绍和快速开始 | 27个实际应用示例 | 完整测试套件说明 | 类和函数参考 | 源代码文件浏览 | 示例程序指南 |

## 🚀 快速访问

### 常用入口点
- **新用户入门**：@ref sample1.cpp "sample1.cpp" → @ref sample2.cpp "sample2.cpp" → @ref sample3.cpp "sample3.cpp"
- **现代 API 学习**：@ref sample16_body_result_api.cpp "sample16" → @ref sample25_xml_style_parser.cpp "sample25"
- **样式系统深入**：@ref sample20_complete_style_system.cpp "sample20" → @ref sample21_style_priority_test.cpp "sample21"
- **文档结构管理**：@ref sample29_simple_outline_demo.cpp "sample29" - 大纲生成和字段TOC

### 核心类文档
- @ref duckx::Document "Document" - 主文档类
- @ref duckx::Body "Body" - 文档主体
- @ref duckx::StyleManager "StyleManager" - 样式管理
- @ref duckx::OutlineManager "OutlineManager" - 文档大纲和目录管理
- @ref duckx::Result "Result<T>" - 现代错误处理

### 测试验证
- **基础功能测试**：test_document.cpp, test_body.cpp
- **样式系统测试**：test_style_manager.cpp, test_style_application.cpp  
- **Result<T> API 测试**：test_result_api.cpp, test_body_result_api.cpp
- **文档结构测试**：test_outline_manager.cpp - 大纲和TOC功能验证

---

<div style="text-align: center; color: #6c757d; font-size: 0.9em; margin-top: 30px;">
💡 <strong>提示</strong>：使用页面顶部的搜索功能可以快速找到特定的类、函数或示例
</div>