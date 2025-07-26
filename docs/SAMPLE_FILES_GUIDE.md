# Sample Files Guide

## Current Active Samples

### Core Style System Demos

**sample20_complete_style_system.cpp** - **Complete Style System Demo** ⭐ **RECOMMENDED**
- 完整的样式应用工作流程演示
- 创建自定义样式和使用内置样式
- 应用样式到段落、文本运行和表格
- 生成正确格式的 DOCX 输出：`sample20_complete_style_system.docx`
- **这是样式系统的主要演示程序**

**sample21_style_priority_test.cpp** - **Style vs Direct Formatting Priority Test**
- 演示样式与直接格式化的优先级关系
- 展示直接格式化如何覆盖样式设置
- 测试多层格式化的交互效果
- 生成输出文件：`sample21_style_priority_test.docx`

### Other Important Samples

**sample1.cpp** - Basic document reading
**sample10.cpp** - Table creation and formatting
**sample15.cpp** - Advanced table formatting with Result<T> API
**sample16_body_result_api.cpp** - Body class Result<T> API demo

### Deprecated Style Samples (历史文件)

以下文件已标记为废弃，使用 sample20_complete_style_system.cpp 替代：
- ~~sample17_deprecated_style_manager.cpp~~ → Use sample20_complete_style_system.cpp
- ~~sample18_deprecated_styled_document.cpp~~ → Use sample20_complete_style_system.cpp  
- ~~sample19_deprecated_style_status.cpp~~ → Use sample20_complete_style_system.cpp

## Usage Recommendation

### For Style System Learning
1. **Start with**: `sample20_complete_style_system.cpp` - 完整的样式系统演示
2. **Advanced**: `sample21_style_priority_test.cpp` - 样式优先级和交互

### For Table Formatting
1. **Basic**: `sample10.cpp` - 基础表格
2. **Advanced**: `sample15.cpp` - 高级表格格式化

### For Result<T> API Learning  
1. **Body API**: `sample16_body_result_api.cpp`
2. **Style API**: `sample20_complete_style_system.cpp` (包含完整的 Result<T> 样式 API)

## File Naming Convention

- `sampleXX.cpp` - 主要功能演示（XX = 数字）
- `sampleXX_specific_name.cpp` - 特定功能演示（仅保留重要的）

## Quick Start

想要快速了解样式系统？运行：
```bash
# Windows
cmake --build cmake-build-debug --target sample20_complete_style_system
cmake-build-debug\samples\sample20_complete_style_system.exe

# Linux/macOS  
make sample20_complete_style_system
./samples/sample20_complete_style_system
```