/*
 * @file: test_table_formatting.cpp
 * @brief: Unit tests for Table formatting methods
 * 
 * Tests the newly added Table, TableRow, and TableCell formatting methods
 * to ensure they work correctly and maintain API consistency.
 */

#include <gtest/gtest.h>
#include "duckx.hpp"
#include <string>
#include <memory>

using namespace duckx;

class TableFormattingTest : public ::testing::Test {
protected:
    void SetUp() override {
        doc = std::make_unique<Document>(Document::create("test_table_formatting.docx"));
        table = doc->body().add_table(3, 3);
    }

    void TearDown() override {
        // Clean up if needed
    }

    std::unique_ptr<Document> doc;
    Table table;
};

// Table formatting tests
TEST_F(TableFormattingTest, TableAlignment) {
    // Test setting and getting table alignment
    table.set_alignment(Alignment::CENTER);
    EXPECT_EQ(table.get_alignment(), Alignment::CENTER);

    table.set_alignment(Alignment::RIGHT);
    EXPECT_EQ(table.get_alignment(), Alignment::RIGHT);

    table.set_alignment(Alignment::LEFT);
    EXPECT_EQ(table.get_alignment(), Alignment::LEFT);
}

TEST_F(TableFormattingTest, TableWidth) {
    // Test setting and getting table width
    double test_width = 400.0;
    table.set_width(test_width);
    
    double retrieved_width;
    EXPECT_TRUE(table.get_width(retrieved_width));
    EXPECT_NEAR(retrieved_width, test_width, 0.1);
}

TEST_F(TableFormattingTest, TableBorders) {
    // Test border style
    std::string test_style = "double";
    table.set_border_style(test_style);
    
    std::string retrieved_style;
    EXPECT_TRUE(table.get_border_style(retrieved_style));
    EXPECT_EQ(retrieved_style, test_style);

    // Test border width
    double test_width = 2.0;
    table.set_border_width(test_width);
    
    double retrieved_width;
    EXPECT_TRUE(table.get_border_width(retrieved_width));
    EXPECT_NEAR(retrieved_width, test_width, 0.1);

    // Test border color
    std::string test_color = "FF0000";
    table.set_border_color(test_color);
    
    std::string retrieved_color;
    EXPECT_TRUE(table.get_border_color(retrieved_color));
    EXPECT_EQ(retrieved_color, test_color);
}

TEST_F(TableFormattingTest, TableCellMargins) {
    // Test cell margins
    double top = 5.0, bottom = 6.0, left = 7.0, right = 8.0;
    table.set_cell_margins(top, bottom, left, right);
    
    double ret_top, ret_bottom, ret_left, ret_right;
    EXPECT_TRUE(table.get_cell_margins(ret_top, ret_bottom, ret_left, ret_right));
    EXPECT_NEAR(ret_top, top, 0.1);
    EXPECT_NEAR(ret_bottom, bottom, 0.1);
    EXPECT_NEAR(ret_left, left, 0.1);
    EXPECT_NEAR(ret_right, right, 0.1);
}

TEST_F(TableFormattingTest, TableChaining) {
    // Test method chaining
    auto& result = table.set_alignment(Alignment::CENTER)
                        .set_width(300)
                        .set_border_style("single")
                        .set_border_width(1.0)
                        .set_border_color("000000");
    
    // Verify chaining returns the same object
    EXPECT_EQ(&result, &table);
    
    // Verify all properties were set
    EXPECT_EQ(table.get_alignment(), Alignment::CENTER);
    
    double width;
    EXPECT_TRUE(table.get_width(width));
    EXPECT_NEAR(width, 300.0, 0.1);
}

// TableRow formatting tests
TEST_F(TableFormattingTest, TableRowHeight) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto& row = *row_it;
        
        // Test row height
        double test_height = 25.0;
        row.set_height(test_height);
        
        double retrieved_height;
        EXPECT_TRUE(row.get_height(retrieved_height));
        EXPECT_NEAR(retrieved_height, test_height, 0.1);
    }
}

TEST_F(TableFormattingTest, TableRowHeightRule) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto& row = *row_it;
        
        // Test height rule
        std::string test_rule = "exact";
        row.set_height_rule(test_rule);
        
        std::string retrieved_rule;
        EXPECT_TRUE(row.get_height_rule(retrieved_rule));
        EXPECT_EQ(retrieved_rule, test_rule);
    }
}

TEST_F(TableFormattingTest, TableRowHeaderSettings) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto& row = *row_it;
        
        // Test header row setting
        row.set_header_row(true);
        EXPECT_TRUE(row.is_header_row());
        
        row.set_header_row(false);
        EXPECT_FALSE(row.is_header_row());
        
        // Test can't split setting
        row.set_cant_split(true);
        EXPECT_TRUE(row.get_cant_split());
        
        row.set_cant_split(false);
        EXPECT_FALSE(row.get_cant_split());
    }
}

// TableCell formatting tests
TEST_F(TableFormattingTest, TableCellWidth) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test cell width
            double test_width = 100.0;
            cell.set_width(test_width);
            
            double retrieved_width;
            EXPECT_TRUE(cell.get_width(retrieved_width));
            EXPECT_NEAR(retrieved_width, test_width, 0.1);
            
            // Test width type
            std::string test_type = "pct";
            cell.set_width_type(test_type);
            
            std::string retrieved_type;
            EXPECT_TRUE(cell.get_width_type(retrieved_type));
            EXPECT_EQ(retrieved_type, test_type);
        }
    }
}

TEST_F(TableFormattingTest, TableCellAlignment) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test vertical alignment
            std::string test_align = "center";
            cell.set_vertical_alignment(test_align);
            
            std::string retrieved_align;
            EXPECT_TRUE(cell.get_vertical_alignment(retrieved_align));
            EXPECT_EQ(retrieved_align, test_align);
        }
    }
}

TEST_F(TableFormattingTest, TableCellBackgroundColor) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test background color
            std::string test_color = "FFFF00";
            cell.set_background_color(test_color);
            
            std::string retrieved_color;
            EXPECT_TRUE(cell.get_background_color(retrieved_color));
            EXPECT_EQ(retrieved_color, test_color);
        }
    }
}

TEST_F(TableFormattingTest, TableCellTextDirection) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test text direction
            std::string test_direction = "tbRl";
            cell.set_text_direction(test_direction);
            
            std::string retrieved_direction;
            EXPECT_TRUE(cell.get_text_direction(retrieved_direction));
            EXPECT_EQ(retrieved_direction, test_direction);
        }
    }
}

TEST_F(TableFormattingTest, TableCellIndividualMargins) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test cell margins
            double top = 2.0, bottom = 3.0, left = 4.0, right = 5.0;
            cell.set_margins(top, bottom, left, right);
            
            double ret_top, ret_bottom, ret_left, ret_right;
            EXPECT_TRUE(cell.get_margins(ret_top, ret_bottom, ret_left, ret_right));
            EXPECT_NEAR(ret_top, top, 0.1);
            EXPECT_NEAR(ret_bottom, bottom, 0.1);
            EXPECT_NEAR(ret_left, left, 0.1);
            EXPECT_NEAR(ret_right, right, 0.1);
        }
    }
}

TEST_F(TableFormattingTest, TableCellBorders) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test border style
            std::string test_style = "dashed";
            cell.set_border_style(test_style);
            
            std::string retrieved_style;
            EXPECT_TRUE(cell.get_border_style(retrieved_style));
            EXPECT_EQ(retrieved_style, test_style);
            
            // Test border width
            double test_width = 1.5;
            cell.set_border_width(test_width);
            
            double retrieved_width;
            EXPECT_TRUE(cell.get_border_width(retrieved_width));
            EXPECT_NEAR(retrieved_width, test_width, 0.1);
            
            // Test border color
            std::string test_color = "00FF00";
            cell.set_border_color(test_color);
            
            std::string retrieved_color;
            EXPECT_TRUE(cell.get_border_color(retrieved_color));
            EXPECT_EQ(retrieved_color, test_color);
        }
    }
}

TEST_F(TableFormattingTest, TableCellChaining) {
    auto rows = table.rows();
    auto row_it = rows.begin();
    if (row_it != rows.end()) {
        auto cells = row_it->cells();
        auto cell_it = cells.begin();
        if (cell_it != cells.end()) {
            auto& cell = *cell_it;
            
            // Test method chaining
            auto& result = cell.set_width(80)
                              .set_vertical_alignment("center")
                              .set_background_color("CCCCCC")
                              .set_border_style("single")
                              .set_border_width(1.0);
            
            // Verify chaining returns the same object
            EXPECT_EQ(&result, &cell);
            
            // Verify properties were set
            double width;
            EXPECT_TRUE(cell.get_width(width));
            EXPECT_NEAR(width, 80.0, 0.1);
            
            std::string align;
            EXPECT_TRUE(cell.get_vertical_alignment(align));
            EXPECT_EQ(align, "center");
        }
    }
}

// Integration test
TEST_F(TableFormattingTest, CompleteTableFormatting) {
    // Format the entire table comprehensively
    table.set_alignment(Alignment::CENTER)
         .set_width(500)
         .set_border_style("single")
         .set_border_width(1.0)
         .set_border_color("000000")
         .set_cell_margins(5, 5, 10, 10);
    
    // Format each row
    int row_index = 0;
    for (auto& row : table.rows()) {
        if (row_index == 0) {
            row.set_height(30)
               .set_height_rule("exact")
               .set_header_row(true);
        } else {
            row.set_height(25)
               .set_height_rule("atLeast");
        }
        
        // Format each cell in the row
        int cell_index = 0;
        for (auto& cell : row.cells()) {
            if (row_index == 0) {
                cell.set_background_color("E0E0E0")
                    .set_vertical_alignment("center")
                    .set_border_style("double");
            } else {
                cell.set_vertical_alignment("top")
                    .set_margins(3, 3, 8, 8);
                
                if (row_index % 2 == 0) {
                    cell.set_background_color("F8F8F8");
                }
            }
            cell_index++;
        }
        row_index++;
    }
    
    // Verify table properties
    EXPECT_EQ(table.get_alignment(), Alignment::CENTER);
    
    double width;
    EXPECT_TRUE(table.get_width(width));
    EXPECT_NEAR(width, 500.0, 0.1);
    
    std::string border_style;
    EXPECT_TRUE(table.get_border_style(border_style));
    EXPECT_EQ(border_style, "single");
}

// Test error handling and edge cases
TEST_F(TableFormattingTest, EdgeCases) {
    // Test with zero/negative values
    table.set_width(0);
    double width;
    EXPECT_TRUE(table.get_width(width));
    EXPECT_NEAR(width, 0.0, 0.1);
    
    // Test with very large values
    table.set_width(10000);
    EXPECT_TRUE(table.get_width(width));
    EXPECT_NEAR(width, 10000.0, 0.1);
    
    // Test with empty strings
    table.set_border_style("");
    std::string style;
    EXPECT_TRUE(table.get_border_style(style));
    EXPECT_EQ(style, "");
}
