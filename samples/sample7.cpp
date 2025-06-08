/*
 * @file: sample7_paragraph_layout.cpp
 * @brief: A sample demonstrating balanced use of fluent API for paragraph formatting.
 *
 * @author: liuye
 * @date: 2025.06.10
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include <locale>
#include "duckx.hpp"

int main()
{
    try
    {
        std::locale::global(std::locale(""));
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Warning: Could not set system default locale. " << e.what() << std::endl;
    }

    try
    {
        duckx::Document doc = duckx::Document::create("sample7_paragraph_layout.docx");
        auto& body = doc.body();

        body.add_paragraph("Sample 7: Paragraph Layout and Formatting")
                .set_alignment(duckx::Alignment::CENTER)
                .add_run("", duckx::bold).set_font_size(16);

        body.add_paragraph();

        // --- 测试 1: 段落间距 ---
        // 创建段落，然后用链式调用设置间距
        body.add_paragraph("This paragraph has extra spacing before and after it.")
                .set_spacing(12, 18);

        body.add_paragraph("Notice the large gap between this paragraph and the one above.");
        body.add_paragraph();

        // --- 测试 2: 行距 ---
        const std::string multi_line_text = "This paragraph is set to double line spacing. Each line inside this "
                "paragraph will have a significant amount of vertical space between it and "
                "the next, making the text much easier to read.";
        body.add_paragraph(multi_line_text)
                .set_line_spacing(2.0);

        body.add_paragraph();

        // --- 测试 3: 段落缩进 ---
        body.add_paragraph("The following quote is indented from both the left and right margins:");

        const std::string quote = "\"The art of writing is the art of discovering what you believe.\" - Gustave Flaubert";
        body.add_paragraph(quote)
                .set_indentation(36, 36)
                .add_run("", duckx::italic);

        body.add_paragraph();

        // --- 测试 4: 首行缩进 ---
        const std::string chinese_text = u8"这是一个典型的中文段落，它应用了首行缩进。在中文排版中，每个段落的开头空两格是一种非常普遍的格式规范，这使得段落的开始非常清晰。";
        body.add_paragraph(chinese_text)
                .set_first_line_indent(24);

        body.add_paragraph();

        // --- 测试 5: 悬挂缩进 ---
        // 这里的操作比较多，分解为变量更清晰，但也可以用链式调用
        body.add_paragraph()
                .add_run("Hanging Indent: ", duckx::bold);

        body.add_paragraph()
                .set_indentation(36, 0)
                .set_first_line_indent(-36)
                .add_run("This is used for definitions or bibliographies. The first line is aligned with the margin, "
                    "while the rest of the paragraph is indented. This makes the leading term stand out.");

        // 保存文档
        doc.save();

        std::cout << "Successfully created 'sample7_paragraph_layout.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
