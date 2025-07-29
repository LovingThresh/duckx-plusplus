# 示例程序总览 {#samples_page}

本页面提供所有示例程序的完整索引和说明。

## 基础功能示例 (Samples 1-9)

这些示例适合初学者了解 DuckX-PLusPlus 的基础功能：

### 文档基础操作
- **@ref sample1.cpp** - 基础文档读取和内容提取
- **@ref sample2.cpp** - 文档创建和基础写入操作  
- **@ref sample3.cpp** - 图像插入和媒体管理

### 文本格式化
- **@ref sample4.cpp** - 内联文本框和富文本格式化
- **@ref sample5.cpp** - 文本对齐和段落布局
- **@ref sample6.cpp** - 字体格式化和字符样式
- **@ref sample7.cpp** - 段落布局和间距控制
- **@ref sample8.cpp** - 列表创建和格式化（项目符号、编号）
- **@ref sample9.cpp** - 文本高亮和颜色效果

## 中级功能示例 (Samples 10-15)

这些示例展示更复杂的功能和文档结构管理：

### 表格操作
- **@ref sample10.cpp** - 表格创建和基础操作
- **@ref sample15.cpp** - 高级表格格式化和样式应用

### 文档结构管理
- **@ref sample11.cpp** - 页眉页脚管理和多种类型支持
- **@ref sample12.cpp** - 关系ID冲突处理和文档完整性
- **@ref sample13.cpp** - 超链接创建和管理
- **@ref sample14.cpp** - 文档分析和内容检查工具

## 高级功能示例 (Samples 16-27)

这些示例展示库的高级特性和现代 API：

### 现代 Result<T> API
- **@ref sample16_body_result_api.cpp** - Body 类 Result<T> API 综合演示
- **@ref sample25_xml_style_parser.cpp** - XML 样式解析器 Result<T> API
- **@ref sample26_style_set_usage.cpp** - StyleSet 使用和错误处理
- **@ref sample27_technical_document_demo.cpp** - 技术文档生成演示

### 样式管理系统
- **@ref sample17_deprecated_style_manager.cpp** - 样式管理器遗留 API 演示
- **@ref sample18_deprecated_styled_document.cpp** - 样式化文档遗留功能
- **@ref sample19_deprecated_style_status.cpp** - 样式状态管理遗留模式
- **@ref sample20_complete_style_system.cpp** - 完整样式系统演示
- **@ref sample21_style_priority_test.cpp** - 样式优先级测试和验证
- **@ref sample22_style_reading_system.cpp** - 样式读取系统演示
- **@ref sample23_complete_style_system.cpp** - 完整样式系统高级用法
- **@ref sample24_style_system_test.cpp** - 样式系统综合测试

### 综合应用
- **@ref sample_comprehensive_test.cpp** - 综合功能测试和使用模式

## 如何使用示例

### 1. 构建示例程序
```bash
# 启用示例构建
cmake -DBUILD_SAMPLES=ON ..
cmake --build .

# 运行示例
cd samples
./sample1
./sample10
# ... 其他示例
```

### 2. 学习路径建议

**新手入门路径**：
1. 从 sample1-3 开始，学习基础文档操作
2. 尝试 sample4-9，掌握文本格式化
3. 学习 sample10、sample15 的表格操作

**进阶开发路径**：
1. 重点学习 sample16+ 的 Result<T> API
2. 深入研究 sample20-24 的完整样式系统
3. 掌握现代错误处理模式和最佳实践

**专业开发路径**：
1. sample25-27 的高级特性集成
2. sample_comprehensive_test 的完整工作流
3. 结合测试程序理解内部实现

### 3. 代码模式参考

每个示例都展示了特定的编程模式：

- **错误处理模式**：Exception API vs Result<T> API
- **资源管理模式**：RAII 和智能指针使用
- **函数式编程**：`and_then()` 和 `or_else()` 链式调用
- **样式系统集成**：从基础样式到完整主题应用

## 相关资源

- **API 文档**：查看具体类的文档以了解详细参数
- **测试程序**：@ref tests_page "测试程序页面" 提供更多使用示例
- **错误处理指南**：学习 @ref duckx::Result "Result<T>" 模式的最佳实践