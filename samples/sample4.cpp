/*
 * @file: sample4_inline_textbox.cpp
 * @brief: A sample demonstrating how to add and manipulate inline TextBoxes.
 *
 * @author: liuye
 * @date: 2025.06.08
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include "duckx.hpp"

int main()
{
    try
    {
        // 1. 创建一个新的 .docx 文件
        duckx::Document doc = duckx::Document::create("sample4_inline_textbox_rich.docx");
        auto& body = doc.body();

        body.add_paragraph("Sample 4: Rich Inline TextBox Functionality").add_run("", duckx::bold);
        body.add_paragraph("This sample demonstrates adding multiple inline textboxes with formatted content.");
        body.add_paragraph(); // 添加一个空段落作为分隔

        // ==========================================================
        //  场景 1: 在一个段落中混合普通文本和文本框
        // ==========================================================
        auto p1 = body.add_paragraph("Here is the first inline textbox -> ");

        // 创建第一个 TextBox 实例
        duckx::TextBox textbox1;
        textbox1.set_size(200, 50); // 设置尺寸

        textbox1.add_paragraph("I am box #1.").add_run(" With bold text.", duckx::bold);

        doc.media().add_textbox(p1, textbox1);

        p1.add_run(" <- a little more text, and then a second textbox -> ");

        // 创建第二个 TextBox 实例
        duckx::TextBox textbox2;
        textbox2.set_size(150, 75); // 不同尺寸

        // 在第二个文本框中添加多段内容
        textbox2.add_new_paragraph("Box #2, line 1.");
        textbox2.add_paragraph("Line 2 is ").add_run("italic.", duckx::italic);

        doc.media().add_textbox(p1, textbox2);

        p1.add_run(" <- and that's the end of the line.");

        body.add_paragraph(); // 添加一个空段落作为分隔

        // ==========================================================
        //  场景 2: 将文本框作为列表项的一部分
        // ==========================================================
        body.add_paragraph("You can even use textboxes in lists:");
        auto p2 = body.add_paragraph("1. First item is plain text.");
        auto p3 = body.add_paragraph("2. Second item contains a summary in a box: ");

        duckx::TextBox summary_box;
        summary_box.set_size(300, 40);
        summary_box.set_border(duckx::BorderStyle::SOLID);
        summary_box.add_paragraph("This is a summary.").add_run(" All rights reserved.", duckx::smallcaps);

        doc.media().add_textbox(p3, summary_box);

        // 4. 保存文档
        doc.save();

        std::cout << "Successfully created 'sample4_inline_textbox_rich.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
