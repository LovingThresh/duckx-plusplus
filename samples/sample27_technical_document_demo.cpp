/*!
 * @file sample27_technical_document_demo.cpp
 * @brief Demonstration of technical documentation creation using predefined styles
 * 
 * Creates a comprehensive technical document about C++ move semantics using
 * the technical_documentation.xml style set. Showcases the complete style
 * system with real-world technical content.
 * 
 * @date 2025.07
 */

#include <iostream>
#include <memory>
#include "Document.hpp"
#include "StyleManager.hpp"
#include "XmlStyleParser.hpp"
#include "test_utils.hpp"

using namespace duckx;

int main()
{
    try
    {
        std::cout << "Creating technical documentation: C++ Move Semantics Guide" << std::endl;

        // 1. Create new document
        auto doc_result = Document::create_safe(test_utils::get_temp_path("technical_move_semantics_guide.docx"));
        if (!doc_result.ok())
        {
            std::cerr << "Failed to create document: " << doc_result.error().to_string() << std::endl;
            return 1;
        }
        Document& doc = doc_result.value();

        // 2. Load technical documentation styles and style sets
        StyleManager& style_mgr = doc.styles();
        XmlStyleParser parser;
        // First load the individual styles from temp directory (copied by CMake)
        std::string xml_path = test_utils::get_temp_path("technical_documentation.xml");
        auto styles_result = parser.load_styles_from_file_safe(xml_path);
        if (!styles_result.ok())
        {
            std::cerr << "Failed to load styles: " << styles_result.error().to_string() << std::endl;
            return 1;
        }
        // Create styles in the StyleManager using the create methods
        for (const auto& loaded_style: styles_result.value())
        {
            // Create a new style of the appropriate type
            Result<Style*> new_style_result = Error(ErrorCategory::GENERAL, ErrorCode::UNKNOWN_ERROR,
                                                    "Unhandled style type");
            switch (loaded_style->type())
            {
                case StyleType::PARAGRAPH:
                    new_style_result = style_mgr.create_paragraph_style_safe(loaded_style->name());
                    break;
                case StyleType::CHARACTER:
                    new_style_result = style_mgr.create_character_style_safe(loaded_style->name());
                    break;
                case StyleType::MIXED:
                    new_style_result = style_mgr.create_mixed_style_safe(loaded_style->name());
                    break;
                case StyleType::TABLE:
                    new_style_result = style_mgr.create_table_style_safe(loaded_style->name());
                    break;
                default:
                    continue; // Skip unknown types
            }
            if (new_style_result.ok())
            {
                Style* new_style = new_style_result.value();
                // Copy properties from loaded style to new style
                new_style->set_paragraph_properties_safe(loaded_style->paragraph_properties());
                new_style->set_character_properties_safe(loaded_style->character_properties());
                new_style->set_table_properties_safe(loaded_style->table_properties());
            }
        }

        // Then load the style sets
        auto style_sets_result = parser.load_style_sets_from_file_safe(xml_path);
        if (!style_sets_result.ok())
        {
            std::cerr << "Failed to load style sets: " << style_sets_result.error().to_string() << std::endl;
            return 1;
        }

        // Register the style sets
        for (const auto& style_set: style_sets_result.value())
        {
            auto register_result = style_mgr.register_style_set_safe(style_set);
            if (!register_result.ok())
            {
                std::cerr << "Failed to register style set '" << style_set.name << "': " << register_result.error().
                        to_string() << std::endl;
            }
        }

        // 3. Apply the technical documentation style set
        auto apply_result = style_mgr.apply_style_set_safe("TechnicalDocumentation", doc);
        if (!apply_result.ok())
        {
            std::cerr << "Failed to apply style set: " << apply_result.error().to_string() << std::endl;
        }

        // 5. Get document body for content creation
        Body& body = doc.body();

        // 6. Create the technical document content

        // Document Title
        auto title_result = body.add_paragraph_safe("C++ Move Semantics: A Complete Guide");
        if (title_result.ok())
        {
            style_mgr.apply_style_safe(title_result.value(), "Tech_Title");
        }

        // Introduction Module
        auto intro_module = body.add_paragraph_safe("1. Introduction");
        if (intro_module.ok())
        {
            style_mgr.apply_style_safe(intro_module.value(), "Tech_ModuleHeading");
        }

        auto intro_desc = body.add_paragraph_safe(
                "Move semantics, introduced in C++11, provides a mechanism to transfer ownership of resources from temporary objects to other objects, eliminating unnecessary copies and improving performance. This guide covers the fundamental concepts, implementation patterns, and best practices.");
        if (intro_desc.ok())
        {
            style_mgr.apply_style_safe(intro_desc.value(), "Tech_Description");
        }

        // What is Move Semantics section
        auto what_section = body.add_paragraph_safe("1.1 What is Move Semantics?");
        if (what_section.ok())
        {
            style_mgr.apply_style_safe(what_section.value(), "Tech_FunctionHeading");
        }

        auto what_desc = body.add_paragraph_safe(
                "Move semantics allows the resources of an rvalue (temporary object) to be moved rather than copied. This is achieved through move constructors and move assignment operators that take rvalue references (&&) as parameters.");
        if (what_desc.ok())
        {
            style_mgr.apply_style_safe(what_desc.value(), "Tech_BodyText");
        }

        // Code example
        auto code_example = body.add_paragraph_safe(
                "// Basic move constructor example\nclass Resource {\nprivate:\n    int* data;\n    size_t size;\n\npublic:\n    // Move constructor\n    Resource(Resource&& other) noexcept\n        : data(other.data), size(other.size) {\n        other.data = nullptr;\n        other.size = 0;\n    }\n    \n    // Move assignment operator\n    Resource& operator=(Resource&& other) noexcept {\n        if (this != &other) {\n            delete[] data;\n            data = other.data;\n            size = other.size;\n            other.data = nullptr;\n            other.size = 0;\n        }\n        return *this;\n    }\n};");
        if (code_example.ok())
        {
            style_mgr.apply_style_safe(code_example.value(), "Tech_CodeBlock");
        }

        // Rvalue References Module
        auto rvalue_module = body.add_paragraph_safe("2. Rvalue References");
        if (rvalue_module.ok())
        {
            style_mgr.apply_style_safe(rvalue_module.value(), "Tech_ModuleHeading");
        }

        auto rvalue_desc = body.add_paragraph_safe(
                "Rvalue references, denoted by &&, are references that can bind to temporary objects. They enable the implementation of move semantics by distinguishing between lvalues and rvalues at compile time.");
        if (rvalue_desc.ok())
        {
            style_mgr.apply_style_safe(rvalue_desc.value(), "Tech_Description");
        }

        // Function signature examples
        auto lvalue_func = body.add_paragraph_safe("void process(const T& lvalue_ref);  // Binds to lvalues");
        if (lvalue_func.ok())
        {
            style_mgr.apply_style_safe(lvalue_func.value(), "Tech_FunctionSignature");
        }

        auto rvalue_func = body.add_paragraph_safe("void process(T&& rvalue_ref);       // Binds to rvalues");
        if (rvalue_func.ok())
        {
            style_mgr.apply_style_safe(rvalue_func.value(), "Tech_FunctionSignature");
        }

        // std::move section
        auto move_section = body.add_paragraph_safe("2.1 std::move Function");
        if (move_section.ok())
        {
            style_mgr.apply_style_safe(move_section.value(), "Tech_FunctionHeading");
        }

        auto move_desc = body.add_paragraph_safe(
                "The std::move function is a utility that casts its argument to an rvalue reference, enabling move semantics even for lvalue objects.");
        if (move_desc.ok())
        {
            style_mgr.apply_style_safe(move_desc.value(), "Tech_BodyText");
        }

        auto move_code = body.add_paragraph_safe(
                "// Using std::move to enable move semantics\nstd::vector<int> source = {1, 2, 3, 4, 5};\nstd::vector<int> destination = std::move(source);\n// source is now in a valid but unspecified state");
        if (move_code.ok())
        {
            style_mgr.apply_style_safe(move_code.value(), "Tech_CodeBlock");
        }

        // Warning about moved-from objects
        auto warning = body.add_paragraph_safe(
                "⚠️ Warning: Objects that have been moved from are left in a valid but unspecified state. Only assign to or destroy moved-from objects.");
        if (warning.ok())
        {
            style_mgr.apply_style_safe(warning.value(), "Tech_Warning");
        }

        // Perfect Forwarding Module
        auto forwarding_module = body.add_paragraph_safe("3. Perfect Forwarding");
        if (forwarding_module.ok())
        {
            style_mgr.apply_style_safe(forwarding_module.value(), "Tech_ModuleHeading");
        }

        auto forwarding_desc = body.add_paragraph_safe(
                "Perfect forwarding allows template functions to forward their arguments to other functions while preserving the value category (lvalue or rvalue) of the arguments.");
        if (forwarding_desc.ok())
        {
            style_mgr.apply_style_safe(forwarding_desc.value(), "Tech_Description");
        }

        // Template function signature
        auto template_func = body.add_paragraph_safe(
                "template<typename T>\nvoid wrapper(T&& arg) {\n    target_function(std::forward<T>(arg));\n}");
        if (template_func.ok())
        {
            style_mgr.apply_style_safe(template_func.value(), "Tech_FunctionSignature");
        }

        // std::forward explanation
        auto forward_section = body.add_paragraph_safe("3.1 std::forward Function");
        if (forward_section.ok())
        {
            style_mgr.apply_style_safe(forward_section.value(), "Tech_FunctionHeading");
        }

        auto forward_desc = body.add_paragraph_safe(
                "std::forward conditionally casts its argument to an rvalue reference, preserving the original value category of the forwarded argument.");
        if (forward_desc.ok())
        {
            style_mgr.apply_style_safe(forward_desc.value(), "Tech_BodyText");
        }

        // Best Practices Module
        auto practices_module = body.add_paragraph_safe("4. Best Practices");
        if (practices_module.ok())
        {
            style_mgr.apply_style_safe(practices_module.value(), "Tech_ModuleHeading");
        }

        // Practice 1
        auto practice1 = body.add_paragraph_safe("4.1 Always Mark Move Operations as noexcept");
        if (practice1.ok())
        {
            style_mgr.apply_style_safe(practice1.value(), "Tech_FunctionHeading");
        }

        auto practice1_desc = body.add_paragraph_safe(
                "Move constructors and move assignment operators should be marked noexcept to enable optimizations in standard library containers.");
        if (practice1_desc.ok())
        {
            style_mgr.apply_style_safe(practice1_desc.value(), "Tech_BodyText");
        }

        auto practice1_code = body.add_paragraph_safe(
                "Resource(Resource&& other) noexcept;  // Good\nResource& operator=(Resource&& other) noexcept;  // Good");
        if (practice1_code.ok())
        {
            style_mgr.apply_style_safe(practice1_code.value(), "Tech_CodeBlock");
        }

        // Practice 2
        auto practice2 = body.add_paragraph_safe("4.2 Implement Rule of Five");
        if (practice2.ok())
        {
            style_mgr.apply_style_safe(practice2.value(), "Tech_FunctionHeading");
        }

        auto practice2_desc = body.add_paragraph_safe(
                "Classes that manage resources should implement destructor, copy constructor, copy assignment, move constructor, and move assignment.");
        if (practice2_desc.ok())
        {
            style_mgr.apply_style_safe(practice2_desc.value(), "Tech_BodyText");
        }

        // Note about default implementations
        auto note = body.add_paragraph_safe(
                "💡 Note: Consider using = default for trivial implementations and = delete for operations you want to prohibit.");
        if (note.ok())
        {
            style_mgr.apply_style_safe(note.value(), "Tech_Note");
        }

        // API Reference Table (simplified for demo)
        auto table_caption = body.add_paragraph_safe("Table 1: Move Semantics Function Reference");
        if (table_caption.ok())
        {
            style_mgr.apply_style_safe(table_caption.value(), "Tech_ParameterHeading");
        }

        auto table_result = body.add_table_safe(4, 3);
        if (table_result.ok())
        {
            Table& api_table = table_result.value();
            style_mgr.apply_style_safe(api_table, "Tech_APITable");
        }

        // Return values section
        auto returns_section = body.add_paragraph_safe("5. Return Value Optimization");
        if (returns_section.ok())
        {
            style_mgr.apply_style_safe(returns_section.value(), "Tech_ModuleHeading");
        }

        auto returns_desc = body.add_paragraph_safe(
                "Return Value Optimization (RVO) and Named Return Value Optimization (NRVO) can eliminate unnecessary copies even without explicit move semantics.");
        if (returns_desc.ok())
        {
            style_mgr.apply_style_safe(returns_desc.value(), "Tech_ReturnValue");
        }

        // Performance considerations
        auto perf_section = body.add_paragraph_safe("6. Performance Considerations");
        if (perf_section.ok())
        {
            style_mgr.apply_style_safe(perf_section.value(), "Tech_ModuleHeading");
        }

        auto perf_desc = body.add_paragraph_safe(
                "Move operations should be significantly faster than copy operations. For simple types like integers, copying may be as fast as moving, so the benefit is primarily for resource-managing classes.");
        if (perf_desc.ok())
        {
            style_mgr.apply_style_safe(perf_desc.value(), "Tech_Description");
        }

        // 6. Save the document
        auto save_result = doc.save_safe();
        if (!save_result.ok())
        {
            std::cerr << "Failed to save document: " << save_result.error().to_string() << std::endl;
            return 1;
        }

        // 7. Display style set application status
        auto status_result = style_mgr.get_style_set_safe("TechnicalDocumentation");
        if (status_result.ok())
        {
            std::cout << "\n=== Style Set Application Status ===" << std::endl;
            std::cout << "Style Set: TechnicalDocumentation loaded successfully" << std::endl;
            std::cout << "Styles available in the style set" << std::endl;
        }

        std::cout << "\n✅ Technical documentation created successfully!" << std::endl;
        std::cout << "📄 File: technical_move_semantics_guide.docx (saved to temp directory)" << std::endl;
        std::cout << "🎨 Styles applied: Technical Documentation theme" << std::endl;
        std::cout << "\nDocument demonstrates:" << std::endl;
        std::cout << "  • Module and function hierarchical headings" << std::endl;
        std::cout << "  • Code block syntax highlighting" << std::endl;
        std::cout << "  • Function signature formatting" << std::endl;
        std::cout << "  • Warning and note callouts" << std::endl;
        std::cout << "  • Technical description styling" << std::endl;
        std::cout << "  • API reference table layout" << std::endl;

        return 0;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
}
