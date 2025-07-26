/*
 * Sample 16: Body Result<T> API Demonstration
 * 
 * This sample demonstrates the modern Result<T> API for Body class operations,
 * showcasing comprehensive error handling and monadic operations.
 */

#include <iostream>
#include <string>
#include <vector>
#include "duckx.hpp"
#include "test_utils.hpp"

using namespace duckx;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void demonstrate_successful_operations() {
    print_separator("Successful Operations with Result<T> API");
    
    auto doc_result = Document::create_safe(test_utils::get_temp_path("sample16_body_result_api.docx"));
    if (!doc_result.ok()) {
        std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
        return;
    }
    
    auto& doc = doc_result.value();
    auto& body = doc.body();
    
    std::cout << "✅ Document created successfully" << std::endl;
    
    // Test 1: Add paragraphs with different formatting
    std::cout << "\n1. Adding paragraphs with Result<T> API:" << std::endl;
    
    auto para1_result = body.add_paragraph_safe("Welcome to DuckX-PLusPlus Result<T> API!");
    if (para1_result.ok()) {
        std::cout << "   ✅ Plain paragraph added successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add paragraph: " << para1_result.error().to_string() << std::endl;
    }
    
    auto para2_result = body.add_paragraph_safe("This text is bold and important.", bold);
    if (para2_result.ok()) {
        std::cout << "   ✅ Bold paragraph added successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add bold paragraph: " << para2_result.error().to_string() << std::endl;
    }
    
    auto para3_result = body.add_paragraph_safe("Combined formatting example.", bold | italic | underline);
    if (para3_result.ok()) {
        std::cout << "   ✅ Multi-formatted paragraph added successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add multi-formatted paragraph: " << para3_result.error().to_string() << std::endl;
    }
    
    // Test 2: Add tables with validation
    std::cout << "\n2. Adding tables with dimension validation:" << std::endl;
    
    auto table1_result = body.add_table_safe(3, 4);
    if (table1_result.ok()) {
        std::cout << "   ✅ 3x4 table added successfully" << std::endl;
        
        // Demonstrate table access through Result<T>
        auto& table = table1_result.value();
        auto rows = table.rows();
        int row_count = 0;
        for (const auto& row : rows) {
            row_count++;
        }
        std::cout << "   📊 Table has " << row_count << " rows as expected" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add 3x4 table: " << table1_result.error().to_string() << std::endl;
    }
    
    auto table2_result = body.add_table_safe(1, 6);
    if (table2_result.ok()) {
        std::cout << "   ✅ 1x6 table added successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add 1x6 table: " << table2_result.error().to_string() << std::endl;
    }
    
    // Test 3: Unicode and special characters
    std::cout << "\n3. Testing Unicode and special characters:" << std::endl;
    
    auto unicode_result = body.add_paragraph_safe("Unicode test: 中文 🌟 αβγ ñ café");
    if (unicode_result.ok()) {
        std::cout << "   ✅ Unicode paragraph added successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add Unicode paragraph: " << unicode_result.error().to_string() << std::endl;
    }
    
    auto special_result = body.add_paragraph_safe("Special chars: !@#$%^&*()_+-=[]{}|;:,.<>?/~`");
    if (special_result.ok()) {
        std::cout << "   ✅ Special characters paragraph added successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to add special characters: " << special_result.error().to_string() << std::endl;
    }
    
    // Save the document
    auto save_result = doc.save_safe();
    if (save_result.ok()) {
        std::cout << "\n💾 Document saved successfully!" << std::endl;
    } else {
        std::cout << "\n❌ Failed to save document: " << save_result.error().to_string() << std::endl;
    }
}

void demonstrate_error_handling() {
    print_separator("Error Handling and Validation");
    
    auto doc_result = Document::create_safe(test_utils::get_temp_path("sample16_error_handling.docx"));
    if (!doc_result.ok()) {
        std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
        return;
    }
    
    auto& doc = doc_result.value();
    auto& body = doc.body();
    
    std::cout << "Testing various error conditions:" << std::endl;
    
    // Test 1: Invalid table dimensions
    std::cout << "\n1. Testing invalid table dimensions:" << std::endl;
    
    auto negative_rows_result = body.add_table_safe(-1, 3);
    if (!negative_rows_result.ok()) {
        std::cout << "   ✅ Correctly caught negative rows error:" << std::endl;
        std::cout << "      Category: " << static_cast<int>(negative_rows_result.error().category()) << std::endl;
        std::cout << "      Code: " << static_cast<int>(negative_rows_result.error().code()) << std::endl;
        std::cout << "      Message: " << negative_rows_result.error().message() << std::endl;
        if (negative_rows_result.error().context().has_value()) {
            std::cout << "      Context: " << negative_rows_result.error().context().value().to_string() << std::endl;
        }
    } else {
        std::cout << "   ❌ Should have failed for negative rows!" << std::endl;
    }
    
    auto negative_cols_result = body.add_table_safe(3, -2);
    if (!negative_cols_result.ok()) {
        std::cout << "   ✅ Correctly caught negative columns error:" << std::endl;
        std::cout << "      Message: " << negative_cols_result.error().message() << std::endl;
    } else {
        std::cout << "   ❌ Should have failed for negative columns!" << std::endl;
    }
    
    // Test 2: Excessive dimensions
    std::cout << "\n2. Testing excessive table dimensions:" << std::endl;
    
    auto huge_table_result = body.add_table_safe(15000, 2);
    if (!huge_table_result.ok()) {
        std::cout << "   ✅ Correctly caught excessive rows error:" << std::endl;
        std::cout << "      Message: " << huge_table_result.error().message() << std::endl;
        if (huge_table_result.error().context().has_value()) {
            std::cout << "      Context info: " << huge_table_result.error().context().value().to_string() << std::endl;
        }
    } else {
        std::cout << "   ❌ Should have failed for excessive rows!" << std::endl;
    }
    
    // Test 3: Text length validation
    std::cout << "\n3. Testing text length validation:" << std::endl;
    
    std::string huge_text(1500000, 'A'); // 1.5MB text
    auto huge_text_result = body.add_paragraph_safe(huge_text);
    if (!huge_text_result.ok()) {
        std::cout << "   ✅ Correctly caught excessive text length error:" << std::endl;
        std::cout << "      Message: " << huge_text_result.error().message() << std::endl;
        
        if (huge_text_result.error().context().has_value()) {
            const auto& context = huge_text_result.error().context().value();
            if (context.additional_info.count("text_length") > 0) {
                std::cout << "      Text length: " << context.additional_info.at("text_length") << " characters" << std::endl;
            }
        }
    } else {
        std::cout << "   ❌ Should have failed for excessive text length!" << std::endl;
    }
    
    // Test 4: Invalid body operations
    std::cout << "\n4. Testing operations on invalid body:" << std::endl;
    
    Body invalid_body; // Default constructor creates invalid body
    auto invalid_para_result = invalid_body.add_paragraph_safe("This should fail");
    if (!invalid_para_result.ok()) {
        std::cout << "   ✅ Correctly caught invalid body error for paragraph:" << std::endl;
        std::cout << "      Message: " << invalid_para_result.error().message() << std::endl;
    } else {
        std::cout << "   ❌ Should have failed for invalid body!" << std::endl;
    }
    
    auto invalid_table_result = invalid_body.add_table_safe(2, 2);
    if (!invalid_table_result.ok()) {
        std::cout << "   ✅ Correctly caught invalid body error for table:" << std::endl;
        std::cout << "      Message: " << invalid_table_result.error().message() << std::endl;
    } else {
        std::cout << "   ❌ Should have failed for invalid body!" << std::endl;
    }
}

void demonstrate_monadic_operations() {
    print_separator("Monadic Operations (and_then, or_else)");
    
    auto doc_result = Document::create_safe(test_utils::get_temp_path("sample16_monadic_operations.docx"));
    if (!doc_result.ok()) {
        std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
        return;
    }
    
    auto& doc = doc_result.value();
    auto& body = doc.body();
    
    std::cout << "Testing monadic operations for functional error handling:" << std::endl;
    
    // Test 1: Successful chain with and_then
    std::cout << "\n1. Testing successful and_then chain:" << std::endl;
    
    auto para_result = body.add_paragraph_safe("Introduction paragraph");
    if (para_result.ok()) {
        std::cout << "   ✅ First paragraph added, now adding table..." << std::endl;
    }
    auto chain_result = body.add_table_safe(2, 3);
    
    if (chain_result.ok()) {
        std::cout << "   ✅ Monadic chain completed successfully!" << std::endl;
        std::cout << "   📊 Final result is a table with valid node" << std::endl;
    } else {
        std::cout << "   ❌ Monadic chain failed: " << chain_result.error().to_string() << std::endl;
    }
    
    // Test 2: Error propagation in chain
    std::cout << "\n2. Testing error propagation in and_then chain:" << std::endl;
    
    Body invalid_body;
    auto error_chain_result = invalid_body.add_paragraph_safe("This will fail");
    // Since invalid_body is not initialized, this will fail
    
    if (!error_chain_result.ok()) {
        std::cout << "   ✅ Error correctly propagated through chain" << std::endl;
        std::cout << "   🔍 Error: " << error_chain_result.error().message() << std::endl;
    } else {
        std::cout << "   ❌ Error should have been propagated!" << std::endl;
    }
    
    // Test 3: Successful or_else (no fallback needed)
    std::cout << "\n3. Testing or_else with successful operation:" << std::endl;
    
    auto or_else_success = body.add_paragraph_safe("This will succeed")
        .or_else([&body](const Error& error) {
            std::cout << "   This fallback should not execute!" << std::endl;
            return body.add_paragraph_safe("Fallback paragraph");
        });
    
    if (or_else_success.ok()) {
        std::cout << "   ✅ or_else handled successful operation correctly (no fallback used)" << std::endl;
    } else {
        std::cout << "   ❌ or_else should have succeeded!" << std::endl;
    }
    
    // Test 4: Fallback with or_else
    std::cout << "\n4. Testing or_else with fallback:" << std::endl;
    
    auto or_else_fallback = invalid_body.add_paragraph_safe("This will fail")
        .or_else([&body](const Error& error) {
            std::cout << "   ⚠️  Primary operation failed, using fallback..." << std::endl;
            std::cout << "   🔍 Original error: " << error.message() << std::endl;
            return body.add_paragraph_safe("Fallback paragraph created successfully!");
        });
    
    if (or_else_fallback.ok()) {
        std::cout << "   ✅ or_else fallback executed successfully!" << std::endl;
    } else {
        std::cout << "   ❌ or_else fallback should have succeeded: " << or_else_fallback.error().to_string() << std::endl;
    }
    
    // Save the document
    auto save_result = doc.save_safe();
    if (save_result.ok()) {
        std::cout << "\n💾 Document saved successfully!" << std::endl;
    } else {
        std::cout << "\n❌ Failed to save document: " << save_result.error().to_string() << std::endl;
    }
}

void demonstrate_performance_and_limits() {
    print_separator("Performance and Limits Testing");
    
    auto doc_result = Document::create_safe(test_utils::get_temp_path("sample16_performance.docx"));
    if (!doc_result.ok()) {
        std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
        return;
    }
    
    auto& doc = doc_result.value();
    auto& body = doc.body();
    
    std::cout << "Testing performance and boundary conditions:" << std::endl;
    
    // Test 1: Add many paragraphs
    std::cout << "\n1. Adding 100 paragraphs:" << std::endl;
    
    int successful_paragraphs = 0;
    for (int i = 0; i < 100; ++i) {
        std::string text = "Paragraph number " + std::to_string(i + 1) + " with some content to test performance.";
        auto para_result = body.add_paragraph_safe(text);
        if (para_result.ok()) {
            successful_paragraphs++;
        } else {
            std::cout << "   ❌ Failed to add paragraph " << (i + 1) << ": " << para_result.error().to_string() << std::endl;
            break;
        }
    }
    std::cout << "   ✅ Successfully added " << successful_paragraphs << " paragraphs" << std::endl;
    
    // Test 2: Add tables at boundary limits
    std::cout << "\n2. Testing boundary limits for tables:" << std::endl;
    
    // Test maximum allowed dimensions
    auto max_table_result = body.add_table_safe(100, 50); // Well within limits
    if (max_table_result.ok()) {
        std::cout << "   ✅ Large table (100x50) created successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to create large table: " << max_table_result.error().to_string() << std::endl;
    }
    
    // Test zero dimensions (edge case)
    auto zero_table_result = body.add_table_safe(0, 0);
    if (zero_table_result.ok()) {
        std::cout << "   ✅ Zero-dimension table handled gracefully" << std::endl;
    } else {
        std::cout << "   ⚠️ Zero-dimension table rejected: " << zero_table_result.error().to_string() << std::endl;
    }
    
    // Test 3: Large but reasonable text
    std::cout << "\n3. Testing large text handling:" << std::endl;
    
    std::string large_text(50000, 'X'); // 50KB text
    large_text += " - This is a large text block for testing.";
    
    auto large_text_result = body.add_paragraph_safe(large_text);
    if (large_text_result.ok()) {
        std::cout << "   ✅ Large text block (50KB) processed successfully" << std::endl;
    } else {
        std::cout << "   ❌ Failed to process large text: " << large_text_result.error().to_string() << std::endl;
    }
    
    // Save the document
    auto save_result = doc.save_safe();
    if (save_result.ok()) {
        std::cout << "\n💾 Performance test document saved successfully!" << std::endl;
    } else {
        std::cout << "\n❌ Failed to save performance test document: " << save_result.error().to_string() << std::endl;
    }
}

int main() {
    print_separator("DuckX-PLusPlus Body Result<T> API Demonstration");
    
    std::cout << "This sample demonstrates the modern Result<T> API for Body class operations." << std::endl;
    std::cout << "Key features:" << std::endl;
    std::cout << "• Comprehensive error handling with detailed context" << std::endl;
    std::cout << "• Input validation and resource limits" << std::endl;
    std::cout << "• Monadic operations (and_then, or_else)" << std::endl;
    std::cout << "• Type-safe error propagation" << std::endl;
    
    try {
        demonstrate_successful_operations();
        demonstrate_error_handling();
        demonstrate_monadic_operations();
        demonstrate_performance_and_limits();
        
        print_separator("Sample Completed Successfully");
        std::cout << "All tests completed! Check the generated DOCX files:" << std::endl;
        std::cout << "• sample16_body_result_api.docx - Main functionality demo" << std::endl;
        std::cout << "• sample16_error_handling.docx - Error handling demo" << std::endl;
        std::cout << "• sample16_monadic_operations.docx - Monadic operations demo" << std::endl;
        std::cout << "• sample16_performance.docx - Performance and limits demo" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n💥 Unexpected exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}