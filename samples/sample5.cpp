/*
 * @file: sample5_alignment.cpp
 * @brief: A sample demonstrating how to set paragraph alignment.
 *
 * @author: liuye
 * @date: 2025.06.09
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>
#include "duckx.hpp"

int main()
{
    try
    {
        // 1. 创建一个新的 .docx 文件
        duckx::Document doc = duckx::Document::create("sample5_alignment_test.docx");
        auto& body = doc.body();

        // ==========================================================
        //  测试 1: 居中对齐 (Center Alignment)
        // ==========================================================
        body.add_paragraph("Document Title")
            .set_alignment(duckx::Alignment::CENTER)
            .add_run("", duckx::bold | duckx::underline); // 同时也应用了格式

        body.add_paragraph(); // 空段落作为分隔

        // ==========================================================
        //  测试 2: 右对齐 (Right Alignment)
        // ==========================================================
        body.add_paragraph("Date: 2025-06-09")
            .set_alignment(duckx::Alignment::RIGHT);

        body.add_paragraph();

        // ==========================================================
        //  测试 3: 左对齐 (Left Alignment - 默认)
        // ==========================================================
        body.add_paragraph("This is a standard left-aligned paragraph. "
                           "It is the default alignment for all new paragraphs, "
                           "so no explicit setting is needed, but we can set it for clarity.")
            .set_alignment(duckx::Alignment::LEFT);

        body.add_paragraph();

        // ==========================================================
        //  测试 4: 两端对齐 (Justified/Both Alignment)
        // ==========================================================
        const std::string long_text = "This is a long paragraph designed to showcase justified alignment. "
                                "When the text is justified, Word adjusts the spacing between words so that "
                                "the text fills the entire width of the line, creating clean, block-like "
                                "edges on both the left and right sides. This is commonly used in newspapers and books.";

        body.add_paragraph(long_text)
            .set_alignment(duckx::Alignment::BOTH);

        body.add_paragraph();

        // ==========================================================
        //  测试 5: 在 TextBox 内部设置对齐
        // ==========================================================
        const auto p_for_box = body.add_paragraph("The following textbox also contains aligned paragraphs:");

        duckx::TextBox textbox(duckx::BorderStyle::SOLID);
        textbox.set_size(400, 100);

        // 在文本框内添加一个居中的段落
        textbox.add_paragraph("Centered text inside a box")
            .set_alignment(duckx::Alignment::CENTER);

        // 在文本框内添加一个右对齐的段落
        textbox.add_paragraph("Right-aligned inside")
            .set_alignment(duckx::Alignment::RIGHT);

        // 添加一个有内容的普通（左对齐）段落
        textbox.add_paragraph("This is a standard left-aligned paragraph within the textbox.")
            .set_alignment(duckx::Alignment::LEFT);

        doc.media().add_textbox(p_for_box, textbox);

        // 6. 保存文档
        doc.save();

        std::cout << "Successfully created 'sample5_alignment_test.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}