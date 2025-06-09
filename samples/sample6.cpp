/*
 * @file: sample6_font_formatting.cpp
 * @brief: A sample demonstrating how to set font, size, and color for text runs.
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
        // 尝试设置UTF-8 locale
        std::locale::global(std::locale("en_US.UTF-8"));
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Warning: Could not set UTF-8 locale. " << e.what() << std::endl;
        std::locale::global(std::locale(""));
    }

    try
    {
        duckx::Document doc = duckx::Document::create("sample6_font_formatting.docx");
        auto& body = doc.body();

        body.add_paragraph("Sample 6: Font Formatting")
            .set_alignment(duckx::Alignment::CENTER)
            .add_run("", duckx::bold | duckx::underline)
            .set_font_size(20);

        body.add_paragraph();

        // ==========================================================
        //  测试 1: 在一个段落中混合多种字体格式
        // ==========================================================
        auto p1 = body.add_paragraph("This paragraph showcases various font styles. ");

        p1.add_run("Red text")
          .set_color("FF0000");
        p1.add_run(" in Times New Roman, ").set_font("Times New Roman");
        p1.add_run("size 16, ").set_font_size(16);
        p1.add_run("and green bold text.").set_color("008000");
        body.add_paragraph();

        // ==========================================================
        //  测试 2: 中文字体和组合格式
        // ==========================================================
        auto p2 = body.add_paragraph();

        p2.add_run(u8"这是一段中文文本，")
          .set_font(u8"微软雅黑") // Microsoft YaHei
          .set_font_size(12);

        p2.add_run(u8"其中这段是加粗的、紫色的、14号字。")
          .set_font(u8"黑体")   // SimHei
          .set_font_size(14)
          .set_color("800080"); // Purple

        body.add_paragraph();

        // ==========================================================
        //  测试 3: 在 TextBox 中应用字体格式
        // ==========================================================
        auto p_for_box = body.add_paragraph("Font formatting inside a textbox:");

        duckx::TextBox textbox(duckx::BorderStyle::SOLID);
        textbox.set_size(400, 80);

        auto p_in_box = textbox.add_paragraph("This is a ");

        p_in_box.add_run("highlighted message")
                .set_font("Courier New")
                .set_font_size(10.5)
                .set_color("D2691E"); // Chocolate color

        p_in_box.add_run(" inside a box.");

        doc.media().add_textbox(p_for_box, textbox);

        doc.save();

        std::cout << "Successfully created 'sample6_font_formatting.docx'." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}