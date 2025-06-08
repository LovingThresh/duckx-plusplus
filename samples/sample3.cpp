/*
 * @file: sample3.cpp
 * @brief:
 *
 * @author: liuye
 * @date: 2025.06.07
 * @copyright (c) 2013-2024 Honghu Yuntu Corporation
 */
#include <iostream>

#include "duckx.hpp"

int main()
{
    try
    {
        duckx::Document doc = duckx::Document::create("sample3_test_with_image.docx");
        const auto p = doc.body().add_paragraph("Company Logo:");
        const duckx::Image logo("logo.png");
        doc.media().add_image(p, logo);
        doc.save();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
