#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstdio>
#include <fstream>

#include "Document.hpp"
#include "Image.hpp"
#include "BaseElement.hpp"

// Helper function to create a minimal valid PNG file for testing
void create_dummy_png_file(const std::string& path) {
    // Create a minimal valid 1x1 PNG file
    const unsigned char png_data[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,  // PNG signature
        0x00, 0x00, 0x00, 0x0D,                          // IHDR chunk length
        0x49, 0x48, 0x44, 0x52,                          // IHDR
        0x00, 0x00, 0x00, 0x01,                          // Width: 1
        0x00, 0x00, 0x00, 0x01,                          // Height: 1
        0x08, 0x02,                                      // Bit depth: 8, Color type: 2 (RGB)
        0x00, 0x00, 0x00,                                // Compression, filter, interlace
        0x90, 0x77, 0x53, 0xDE,                          // CRC
        0x00, 0x00, 0x00, 0x0C,                          // IDAT chunk length
        0x49, 0x44, 0x41, 0x54,                          // IDAT
        0x08, 0x99, 0x01, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, // Image data
        0x02, 0x00, 0x01,                                // CRC
        0x00, 0x00, 0x00, 0x00,                          // IEND chunk length
        0x49, 0x45, 0x4E, 0x44,                          // IEND
        0xAE, 0x42, 0x60, 0x82                           // CRC
    };

    std::ofstream outfile(path, std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(png_data), sizeof(png_data));
    outfile.close();
}

class ImageTest : public ::testing::Test {
protected:
    std::string test_docx_path;
    std::string test_image_path;
    std::unique_ptr<duckx::Document> doc;
    pugi::xml_document xml_doc;
    pugi::xml_node test_run_node;

    void SetUp() override {
        // Use different names to avoid conflicts with the actual test resources
        test_docx_path = "temp_test_image_doc.docx";
        test_image_path = "temp_test_logo.png";

        // Create a valid dummy PNG file for tests (different from the actual logo.png)
        create_dummy_png_file(test_image_path);

        // Create a document and a run node to act as a parent for the drawing
        doc = std::make_unique<duckx::Document>(duckx::Document::create(test_docx_path));
        test_run_node = xml_doc.append_child("w:r");
    }

    void TearDown() override {
        // Clean up only the temporary files we created
        doc.reset();
        remove(test_docx_path.c_str());
        remove(test_image_path.c_str());
        // Do NOT delete my_test.docx or logo.png as they are shared test resources
    }

    // Helper to convert pixels to EMUs for verification
    static long long pixels_to_emu(const int pixels) {
        return pixels * 9525;
    }
};

TEST_F(ImageTest, Constructor) {
    ASSERT_NO_THROW({
        const duckx::Image image(test_image_path);
        EXPECT_EQ(image.get_path(), test_image_path);
    });
}

TEST_F(ImageTest, GetPath) {
    const duckx::Image image(test_image_path);
    EXPECT_EQ(image.get_path(), test_image_path);
}

TEST_F(ImageTest, SetSize) {
    duckx::Image image(test_image_path);
    constexpr int width_px = 200;
    constexpr int height_px = 150;

    image.set_size(width_px, height_px);

    // Generate XML and verify the size attributes
    const std::string rid = "rId_test_size";
    constexpr unsigned int drawing_id = 1;
    image.generate_drawing_xml(test_run_node, rid, drawing_id);

    const pugi::xml_node drawing_node = test_run_node.child("w:drawing");
    ASSERT_TRUE(drawing_node);

    const pugi::xml_node extent_node = drawing_node.select_node(".//wp:extent").node();
    ASSERT_TRUE(extent_node);
    EXPECT_EQ(extent_node.attribute("cx").as_llong(), pixels_to_emu(width_px));
    EXPECT_EQ(extent_node.attribute("cy").as_llong(), pixels_to_emu(height_px));
}

TEST_F(ImageTest, RealImageFile_BasicFunctionality) {
    // Try multiple possible paths for the logo.png file
    std::vector<std::string> possible_paths = {
        "logo.png",                                    // Current directory
        "test/logo.png",                              // From build root
        "../test/logo.png",                           // From some subdirectory
        "cmake-build-debug/test/logo.png"             // From project root
    };
    
    std::string real_image_path;
    bool found = false;
    
    for (const auto& path : possible_paths) {
        std::ifstream file_check(path);
        if (file_check.good()) {
            real_image_path = path;
            found = true;
            file_check.close();
            break;
        }
        file_check.close();
    }
    
    if (!found) {
        GTEST_SKIP() << "Real image file 'logo.png' not found in any expected location. Skipping test.";
    }

    // Test constructor with real image
    ASSERT_NO_THROW({
        const duckx::Image real_image(real_image_path);
        EXPECT_EQ(real_image.get_path(), real_image_path);
    });

    // Test that real image can generate valid XML
    duckx::Image real_image(real_image_path);
    const std::string rid = "rId_real_image";
    constexpr unsigned int drawing_id = 5;

    ASSERT_NO_THROW(real_image.generate_drawing_xml(test_run_node, rid, drawing_id));

    const pugi::xml_node drawing_node = test_run_node.child("w:drawing");
    ASSERT_TRUE(drawing_node);

    // Should be inline by default
    const pugi::xml_node inline_node = drawing_node.child("wp:inline");
    ASSERT_TRUE(inline_node);

    // Verify basic structure
    ASSERT_TRUE(inline_node.child("wp:extent"));
    ASSERT_TRUE(inline_node.child("wp:docPr"));
    ASSERT_TRUE(inline_node.child("a:graphic"));

    // Verify the relationship ID is correctly set
    const pugi::xml_node blip_node = inline_node.select_node(".//a:blip").node();
    ASSERT_TRUE(blip_node);
    EXPECT_STREQ(blip_node.attribute("r:embed").value(), rid.c_str());

    // Verify the drawing properties contain the real filename
    const pugi::xml_node cNvPr_node = inline_node.select_node(".//pic:cNvPr").node();
    ASSERT_TRUE(cNvPr_node);
    EXPECT_STREQ(cNvPr_node.attribute("name").value(), "logo.png");
}

TEST_F(ImageTest, RealImageFile_WithAbsolutePositioning) {
    // Try multiple possible paths for the logo.png file
    std::vector<std::string> possible_paths = {
        "logo.png",                                    // Current directory
        "test/logo.png",                              // From build root
        "../test/logo.png",                           // From some subdirectory
        "cmake-build-debug/test/logo.png"             // From project root
    };
    
    std::string real_image_path;
    bool found = false;
    
    for (const auto& path : possible_paths) {
        std::ifstream file_check(path);
        if (file_check.good()) {
            real_image_path = path;
            found = true;
            file_check.close();
            break;
        }
        file_check.close();
    }
    
    if (!found) {
        GTEST_SKIP() << "Real image file 'logo.png' not found in any expected location. Skipping test.";
    }

    duckx::Image real_image(real_image_path);

    // Set custom size and absolute position
    real_image.set_size(300, 200);
    const long long x_emu = pixels_to_emu(75);
    const long long y_emu = pixels_to_emu(100);
    real_image.set_absolute_position(x_emu, y_emu, duckx::RelativeFrom::PAGE, duckx::RelativeFrom::MARGIN);

    const std::string rid = "rId_real_image_positioned";
    constexpr unsigned int drawing_id = 6;

    ASSERT_NO_THROW(real_image.generate_drawing_xml(test_run_node, rid, drawing_id));

    const pugi::xml_node drawing_node = test_run_node.child("w:drawing");
    ASSERT_TRUE(drawing_node);

    // Should be anchor due to absolute positioning
    const pugi::xml_node anchor_node = drawing_node.child("wp:anchor");
    ASSERT_TRUE(anchor_node);

    // Verify positioning attributes
    const pugi::xml_node pos_h_node = anchor_node.child("wp:positionH");
    ASSERT_TRUE(pos_h_node);
    EXPECT_STREQ(pos_h_node.attribute("relativeFrom").value(), "page");
    EXPECT_EQ(pos_h_node.child("wp:posOffset").text().as_llong(), x_emu);

    const pugi::xml_node pos_v_node = anchor_node.child("wp:positionV");
    ASSERT_TRUE(pos_v_node);
    EXPECT_STREQ(pos_v_node.attribute("relativeFrom").value(), "margin");
    EXPECT_EQ(pos_v_node.child("wp:posOffset").text().as_llong(), y_emu);

    // Verify size attributes
    const pugi::xml_node extent_node = anchor_node.child("wp:extent");
    ASSERT_TRUE(extent_node);
    EXPECT_EQ(extent_node.attribute("cx").as_llong(), pixels_to_emu(300));
    EXPECT_EQ(extent_node.attribute("cy").as_llong(), pixels_to_emu(200));

    // Verify the relationship ID and filename
    const pugi::xml_node blip_node = anchor_node.select_node(".//a:blip").node();
    ASSERT_TRUE(blip_node);
    EXPECT_STREQ(blip_node.attribute("r:embed").value(), rid.c_str());

    const pugi::xml_node cNvPr_node = anchor_node.select_node(".//pic:cNvPr").node();
    ASSERT_TRUE(cNvPr_node);
    EXPECT_STREQ(cNvPr_node.attribute("name").value(), "logo.png");
}

TEST_F(ImageTest, SetAbsolutePosition) {
    duckx::Image image(test_image_path);
    const long long x_emu = pixels_to_emu(100);
    const long long y_emu = pixels_to_emu(50);

    // Test that set_absolute_position method can be called without throwing
    ASSERT_NO_THROW(image.set_absolute_position(x_emu, y_emu, duckx::RelativeFrom::MARGIN, duckx::RelativeFrom::PAGE));

    // Generate XML and verify the position attributes
    const std::string rid = "rId_test_pos";
    constexpr unsigned int drawing_id = 2;
    ASSERT_NO_THROW(image.generate_drawing_xml(test_run_node, rid, drawing_id));

    const pugi::xml_node drawing_node = test_run_node.child("w:drawing");
    ASSERT_TRUE(drawing_node);

    // Using absolute position should now create an <wp:anchor> element
    const pugi::xml_node anchor_node = drawing_node.child("wp:anchor");
    ASSERT_TRUE(anchor_node);

    const pugi::xml_node pos_h_node = anchor_node.child("wp:positionH");
    ASSERT_TRUE(pos_h_node);
    EXPECT_STREQ(pos_h_node.attribute("relativeFrom").value(), "margin");
    EXPECT_EQ(pos_h_node.child("wp:posOffset").text().as_llong(), x_emu);

    const pugi::xml_node pos_v_node = anchor_node.child("wp:positionV");
    ASSERT_TRUE(pos_v_node);
    EXPECT_STREQ(pos_v_node.attribute("relativeFrom").value(), "page");
    EXPECT_EQ(pos_v_node.child("wp:posOffset").text().as_llong(), y_emu);
}


TEST_F(ImageTest, GenerateDrawingXml_Inline) {
    duckx::Image image(test_image_path);
    image.set_size(100, 80);

    const std::string rid = "rId_test_inline";
    constexpr unsigned int drawing_id = 3;

    ASSERT_NO_THROW(image.generate_drawing_xml(test_run_node, rid, drawing_id));

    // Verify the basic structure of the <w:drawing> element
    const pugi::xml_node drawing_node = test_run_node.child("w:drawing");
    ASSERT_TRUE(drawing_node);

    // By default, it should be an inline element
    const pugi::xml_node inline_node = drawing_node.child("wp:inline");
    ASSERT_TRUE(inline_node);

    // Check for essential child nodes
    ASSERT_TRUE(inline_node.child("wp:extent"));
    ASSERT_TRUE(inline_node.child("wp:docPr"));
    ASSERT_TRUE(inline_node.child("a:graphic"));

    // Verify the relationship ID is correctly set
    const pugi::xml_node blip_node = inline_node.select_node(".//a:blip").node();
    ASSERT_TRUE(blip_node);
    EXPECT_STREQ(blip_node.attribute("r:embed").value(), rid.c_str());

    // Verify the drawing properties ID
    const pugi::xml_node docpr_node = inline_node.child("wp:docPr");
    ASSERT_TRUE(docpr_node);
    EXPECT_EQ(docpr_node.attribute("id").as_uint(), drawing_id);
}

TEST_F(ImageTest, GenerateDrawingXml_Anchor) {
    duckx::Image image(test_image_path);
    image.set_size(120, 90);
    image.set_absolute_position(pixels_to_emu(50), pixels_to_emu(50));

    const std::string rid = "rId_test_anchor";
    constexpr unsigned int drawing_id = 4;

    ASSERT_NO_THROW(image.generate_drawing_xml(test_run_node, rid, drawing_id));

    // Verify the basic structure of the <w:drawing> element
    const pugi::xml_node drawing_node = test_run_node.child("w:drawing");
    ASSERT_TRUE(drawing_node);

    // Setting absolute position should now create an anchor element
    const pugi::xml_node anchor_node = drawing_node.child("wp:anchor");
    ASSERT_TRUE(anchor_node);

    // Check for essential child nodes
    ASSERT_TRUE(anchor_node.child("wp:simplePos"));
    ASSERT_TRUE(anchor_node.child("wp:positionH"));
    ASSERT_TRUE(anchor_node.child("wp:positionV"));
    ASSERT_TRUE(anchor_node.child("wp:extent"));
    ASSERT_TRUE(anchor_node.child("wp:docPr"));
    ASSERT_TRUE(anchor_node.child("a:graphic"));

    // Verify the relationship ID is correctly set
    const pugi::xml_node blip_node = anchor_node.select_node(".//a:blip").node();
    ASSERT_TRUE(blip_node);
    EXPECT_STREQ(blip_node.attribute("r:embed").value(), rid.c_str());

    // Verify the drawing properties ID
    const pugi::xml_node docpr_node = anchor_node.child("wp:docPr");
    ASSERT_TRUE(docpr_node);
    EXPECT_EQ(docpr_node.attribute("id").as_uint(), drawing_id);
}