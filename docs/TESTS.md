# 测试程序总览 {#tests_page}

本页面提供所有测试程序的完整索引和说明，帮助开发者了解库的测试覆盖范围和使用模式。

## 测试框架架构

- **测试框架**：GoogleTest
- **统一运行器**：`run_gtests` 可执行文件
- **测试覆盖率**：90%+ 目标覆盖率
- **运行方式**：`./test/run_gtests --gtest_brief=1`

## 核心组件测试

### 基础元素测试
- **test_base_element.cpp** - BaseElement 基类功能测试
- **test_body.cpp** - Body 文档主体容器测试
- **test_document.cpp** - Document 主文档类测试
- **test_docxFile.cpp** - DocxFile 低级文件操作测试

### 文档元素测试
- **test_basic.cpp** - 基础功能集成测试
- **test_textBox.cpp** - TextBox 文本框元素测试
- **test_image.cpp** - 图像处理和媒体管理测试
- **test_iterator.cpp** - 文档元素迭代器测试

### 管理器组件测试
- **test_headerFooterManager.cpp** - 页眉页脚管理器测试
- **test_hyperlinkManager.cpp** - 超链接管理器测试

## 现代 API 测试

### Result<T> 错误处理测试
- **test_result_api.cpp** - Result<T> 模式综合测试
- **test_body_result_api.cpp** - Body 类 Result<T> API 专项测试

### 样式系统测试
- **test_style_manager.cpp** - StyleManager 核心功能测试
- **test_style_application.cpp** - 样式应用机制测试
- **test_style_application_implementation.cpp** - 样式应用实现细节测试
- **test_style_implementation_validation.cpp** - 样式实现验证测试
- **test_style_reading.cpp** - 样式读取系统测试
- **test_style_system_basic.cpp** - 样式系统基础功能测试

### 高级样式功能测试
- **test_complete_style_implementation.cpp** - 完整样式实现测试
- **test_style_set_integration.cpp** - StyleSet 集成测试（新增）
- **test_xml_style_parser.cpp** - XML 样式解析器测试
- **test_xml_style_parser_complete.cpp** - XML 样式解析器完整测试（新增）

## 表格功能测试

- **test_table_formatting.cpp** - 表格格式化和布局测试

## 测试分类说明

### 1. 单元测试 (Unit Tests)
测试单个类或组件的功能：
- 每个核心类都有对应的测试文件
- 测试覆盖公有接口的所有方法
- 包括正常情况和边界条件测试

### 2. 集成测试 (Integration Tests)
测试组件间的协作：
- `test_complete_style_implementation.cpp`
- `test_style_set_integration.cpp`
- `test_body_result_api.cpp`

### 3. API 兼容性测试
确保双 API 设计的一致性：
- Exception API vs Result<T> API
- 传统模式 vs 现代模式
- 向后兼容性验证

### 4. 错误处理测试
专门测试错误情况和恢复机制：
- 文件不存在、权限不足等 I/O 错误
- XML 格式错误和解析失败
- 内存不足和资源限制
- 参数验证和输入检查

## 测试数据和资源

### 测试文件管理
- 测试资源自动复制到构建目录
- 临时文件在测试后自动清理
- 支持不同平台的路径处理

### 测试用 DOCX 文件
```
test/resources/
├── sample.docx          # 基础测试文档
├── complex_table.docx   # 复杂表格测试
├── styled_document.docx # 样式化文档测试
└── media_test.docx      # 媒体内容测试
```

## 运行测试

### 完整测试套件
```bash
# 构建测试
cmake -DBUILD_TESTING=ON ..
cmake --build . --target run_gtests

# 运行所有测试
./test/run_gtests

# 简洁输出模式
./test/run_gtests --gtest_brief=1

# 运行特定测试
./test/run_gtests --gtest_filter="*StyleManager*"
```

### 单独测试文件
```bash
# 构建单个测试
cmake --build . --target test_document

# 运行单个测试
./test/test_document
```

### 测试配置选项
```bash
# 详细输出
./test/run_gtests --gtest_verbose

# 重复运行（压力测试）
./test/run_gtests --gtest_repeat=10

# 并行测试
./test/run_gtests --gtest_parallel=4
```

## 测试最佳实践

### 1. 测试命名约定
- `test_[component_name].cpp` - 组件测试
- `Test[ClassName][Functionality]` - 测试类命名
- `Should[ExpectedBehavior]When[Condition]` - 测试方法命名

### 2. 断言模式
```cpp
// 成功情况测试
EXPECT_TRUE(result.ok());
EXPECT_EQ(expected_value, result.value());

// 错误情况测试  
EXPECT_FALSE(result.ok());
EXPECT_EQ(ErrorCategory::VALIDATION, result.error().category());
```

### 3. 资源管理测试
- 每个测试独立创建和清理资源
- 使用 RAII 模式确保资源释放
- 测试内存泄漏和资源泄漏

### 4. 跨平台测试
- 路径分隔符处理
- 文件权限和访问控制
- 字符编码和本地化

## 持续集成

### GitHub Actions 集成
- 多平台自动测试（Windows, Linux, macOS）
- 不同编译器版本测试（MSVC, GCC, Clang）
- 代码覆盖率报告生成
- 内存检查和静态分析

### 本地开发测试
```bash
# 快速验证
make test

# 完整测试套件
ctest --verbose

# 内存检查（Linux）
valgrind --tool=memcheck ./test/run_gtests
```

## 贡献测试代码

### 添加新测试
1. 在 `test/` 目录创建 `test_[feature].cpp`
2. 包含必要的头文件和测试工具
3. 编写测试用例覆盖新功能
4. 更新 `test/CMakeLists.txt`
5. 验证测试通过并提交

### 测试代码质量标准
- **覆盖率**：新功能必须有 90%+ 测试覆盖率
- **可读性**：测试代码要清晰表达测试意图
- **独立性**：测试之间不能有依赖关系
- **性能**：单个测试应在合理时间内完成

## 相关资源

- **示例程序**：@ref samples_page "示例程序页面" 提供实际使用案例
- **API 文档**：查看具体类和方法的详细说明
- **错误处理指南**：了解 @ref duckx::Result "Result<T>" 和 @ref duckx::Error "Error" 类的使用