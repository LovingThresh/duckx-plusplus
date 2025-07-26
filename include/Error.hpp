/*!
 * @file Error.hpp
 * @brief Comprehensive error handling system
 * 
 * Provides modern error handling with categorized error codes, rich context
 * information, and Result<T> monadic operations for robust error management.
 */
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "duckx_export.h"

namespace duckx
{
    // ============================================================================
    // Error Categories and Codes
    // ============================================================================

    /*!
     * @brief High-level error categories for organizing error types
     * 
     * Provides logical grouping of errors to help with error handling
     * and debugging. Each category covers a specific domain of operations.
     */
    enum class ErrorCategory
    {
        GENERAL = 0,
        FILE_IO = 1,
        XML_PARSING = 2,
        DOCX_FORMAT = 3,
        MEDIA_HANDLING = 4,
        ELEMENT_OPERATION = 5,
        VALIDATION = 6,
        RESOURCE = 7,
        STYLE_SYSTEM = 8,
        TEMPLATE_SYSTEM = 9,
        DOCUMENT_COMPARISON = 10,
        ENGINEERING_TOOLS = 11
    };

    enum class ErrorCode
    {
        // General errors (0-99)
        SUCCESS = 0,
        UNKNOWN_ERROR = 1,
        INVALID_ARGUMENT = 2,
        NOT_IMPLEMENTED = 3,
        OPERATION_CANCELLED = 4,

        // File I/O errors (100-199)
        FILE_NOT_FOUND = 100,
        FILE_ACCESS_DENIED = 101,
        FILE_CORRUPTED = 102,
        ZIP_OPERATION_FAILED = 103,
        ENTRY_NOT_FOUND = 104,
        ENTRY_READ_FAILED = 105,
        ENTRY_WRITE_FAILED = 106,

        // XML parsing errors (200-299)
        XML_PARSE_ERROR = 200,
        XML_INVALID_STRUCTURE = 201,
        XML_NODE_NOT_FOUND = 202,
        XML_ATTRIBUTE_MISSING = 203,
        XML_NAMESPACE_ERROR = 204,

        // DOCX format errors (300-399)
        DOCX_INVALID_FORMAT = 300,
        DOCX_MISSING_CONTENT_TYPES = 301,
        DOCX_MISSING_RELATIONSHIPS = 302,
        DOCX_INVALID_DOCUMENT_XML = 303,
        DOCX_UNSUPPORTED_VERSION = 304,

        // Media handling errors (400-499)
        IMAGE_FILE_INVALID = 400,
        IMAGE_FORMAT_UNSUPPORTED = 401,
        IMAGE_SIZE_INVALID = 402,
        MEDIA_RELATIONSHIP_FAILED = 403,

        // Element operation errors (500-599)
        ELEMENT_NOT_FOUND = 500,
        ELEMENT_INVALID_STATE = 501,
        ELEMENT_OPERATION_FAILED = 502,
        ITERATOR_OUT_OF_BOUNDS = 503,
        PARAGRAPH_CREATION_FAILED = 504,
        RUN_CREATION_FAILED = 505,
        TABLE_CREATION_FAILED = 506,

        // Validation errors (600-699)
        VALIDATION_FAILED = 600,
        INVALID_TEXT_LENGTH = 601,
        INVALID_FONT_SIZE = 602,
        INVALID_COLOR_FORMAT = 603,
        INVALID_ALIGNMENT = 604,
        INVALID_SPACING = 605,
        INVALID_TABLE_DIMENSIONS = 606,
        INVALID_BORDER_STYLE = 607,
        INVALID_MARGIN_VALUE = 608,
        INVALID_PADDING_VALUE = 609,
        INVALID_WIDTH_VALUE = 610,
        INVALID_HEIGHT_VALUE = 611,
        INVALID_TEMPLATE_VARIABLE = 612,

        // Resource errors (700-799)
        MEMORY_ALLOCATION_FAILED = 700,
        RESOURCE_LIMIT_EXCEEDED = 701,
        DEPENDENCY_NOT_AVAILABLE = 702,

        // Style system errors (800-899)
        STYLE_NOT_FOUND = 800,
        STYLE_ALREADY_EXISTS = 801,
        STYLE_INVALID_TYPE = 802,
        STYLE_PROPERTY_INVALID = 803,
        STYLE_APPLICATION_FAILED = 804,
        STYLE_INHERITANCE_CYCLE = 805,
        STYLE_TEMPLATE_INVALID = 806,
        STYLE_MANAGER_NOT_INITIALIZED = 807,
        STYLE_XML_GENERATION_FAILED = 808,
        STYLE_DEPENDENCY_MISSING = 809,

        // Template system errors (900-999)
        TEMPLATE_NOT_FOUND = 900,
        TEMPLATE_SYNTAX_ERROR = 901,
        TEMPLATE_VARIABLE_UNDEFINED = 902,
        TEMPLATE_FUNCTION_NOT_REGISTERED = 903,
        TEMPLATE_CONDITION_INVALID = 904,
        TEMPLATE_LOOP_SYNTAX_ERROR = 905,
        TEMPLATE_GENERATION_FAILED = 906,
        TEMPLATE_ENGINE_NOT_INITIALIZED = 907,

        // Document comparison errors (1000-1099)
        COMPARISON_DOCUMENTS_INVALID = 1000,
        COMPARISON_ALGORITHM_FAILED = 1001,
        DIFF_REPORT_GENERATION_FAILED = 1002,
        MERGE_CONFLICT_UNRESOLVABLE = 1003,
        MERGE_STRATEGY_INVALID = 1004,

        // Engineering tools errors (1100-1199)
        REQUIREMENT_TABLE_INVALID = 1100,
        TRACEABILITY_MATRIX_FAILED = 1101,
        TEST_COVERAGE_DATA_INVALID = 1102,
        CODE_BLOCK_FORMAT_UNSUPPORTED = 1103,
        TECHNICAL_FORMAT_INVALID = 1104,
        BATCH_PROCESSING_FAILED = 1105
    };

    // ============================================================================
    // Error Context and Details
    // ============================================================================

    struct DUCKX_API ErrorContext
    {
        std::string file_path;
        std::string function_name;
        int line_number = 0;
        std::string operation;
        std::map<std::string, std::string> additional_info;

        ErrorContext() = default;

        ErrorContext(const absl::string_view file, const absl::string_view func, const int line,
                     const absl::string_view op = "")
            : file_path(file.data(), file.size())
              , function_name(func.data(), func.size())
              , line_number(line)
              , operation(op.data(), op.size()) {}

        ErrorContext& with_info(const absl::string_view key, const absl::string_view value)
        {
            additional_info[std::string(key)] = std::string(value);
            return *this;
        }

        // Convenience methods for common context information
        ErrorContext& with_element_type(const absl::string_view element_type)
        {
            return with_info("element_type", element_type);
        }

        ErrorContext& with_document_path(const absl::string_view doc_path)
        {
            return with_info("document_path", doc_path);
        }

        ErrorContext& with_table_position(int row, int col)
        {
            return with_info("table_row", std::to_string(row))
                   .with_info("table_col", std::to_string(col));
        }

        ErrorContext& with_style_name(const absl::string_view style_name)
        {
            return with_info("style_name", style_name);
        }

        ErrorContext& with_template_name(const absl::string_view template_name)
        {
            return with_info("template_name", template_name);
        }

        ErrorContext& with_validation_rule(const absl::string_view rule_name)
        {
            return with_info("validation_rule", rule_name);
        }

        std::string to_string() const
        {
            std::string result = absl::StrFormat("[%s:%d in %s]", file_path, line_number, function_name);
            if (!operation.empty())
            {
                result += absl::StrFormat(" Operation: %s", operation);
            }
            for (const auto& pair: additional_info)
            {
                result += absl::StrFormat(" %s=%s", pair.first, pair.second);
            }
            return result;
        }
    };

    // Forward declaration for Error class
    class Error;

    // ============================================================================
    // Enhanced Error Class
    // ============================================================================

    class DUCKX_API Error
    {
    public:
        Error()
            : m_category(ErrorCategory::GENERAL), m_code(ErrorCode::SUCCESS) {}

        Error(const ErrorCategory category, const ErrorCode code, const absl::string_view message)
            : m_category(category), m_code(code), m_message(message.data(), message.size()) {}

        Error(const ErrorCategory category, const ErrorCode code, const absl::string_view message,
              const ErrorContext& context)
            : m_category(category), m_code(code), m_message(message.data(), message.size()), m_context(context) {}

        // Copy constructor
        Error(const Error& other)
            : m_category(other.m_category), m_code(other.m_code), m_message(other.m_message), m_context(other.m_context)
        {
            // Deep copy the causes
            for (const auto& cause : other.m_causes) {
                m_causes.push_back(std::make_unique<Error>(*cause));
            }
        }

        // Copy assignment operator
        Error& operator=(const Error& other)
        {
            if (this != &other) {
                m_category = other.m_category;
                m_code = other.m_code;
                m_message = other.m_message;
                m_context = other.m_context;
                
                // Clear existing causes and deep copy new ones
                m_causes.clear();
                for (const auto& cause : other.m_causes) {
                    m_causes.push_back(std::make_unique<Error>(*cause));
                }
            }
            return *this;
        }

        // Move constructor and assignment (default should work)
        Error(Error&&) = default;
        Error& operator=(Error&&) = default;

        // Chain errors for better error tracking
        Error& caused_by(const Error& cause)
        {
            m_causes.push_back(std::make_unique<Error>(cause));
            return *this;
        }

        // Getters
        ErrorCategory category() const { return m_category; }
        ErrorCode code() const { return m_code; }
        const std::string& message() const { return m_message; }
        const absl::optional<ErrorContext>& context() const { return m_context; }
        const std::vector<std::unique_ptr<Error>>& causes() const { return m_causes; }

        // Utility methods
        bool is_success() const { return m_code == ErrorCode::SUCCESS; }
        bool is_file_error() const { return m_category == ErrorCategory::FILE_IO; }
        bool is_xml_error() const { return m_category == ErrorCategory::XML_PARSING; }
        bool is_docx_error() const { return m_category == ErrorCategory::DOCX_FORMAT; }

        // Convert to absl::Status for compatibility
        absl::Status to_status() const
        {
            if (is_success())
            {
                return absl::OkStatus();
            }

            absl::StatusCode status_code = to_absl_status_code();
            std::string full_message = m_message;

            if (m_context.has_value())
            {
                full_message += " " + m_context->to_string();
            }

            return {status_code, full_message};
        }

        // Formatted string representation
        std::string to_string() const // NOLINT(*-no-recursion)
        {
            std::string result = absl::StrFormat("[%s:%d] %s",
                                                 category_to_string(m_category),
                                                 static_cast<int>(m_code),
                                                 m_message);

            if (m_context.has_value())
            {
                result += " " + m_context->to_string();
            }

            // Include cause chain
            for (const auto& cause: m_causes)
            {
                result += "\n  Caused by: " + cause->to_string();
            }

            return result;
        }

    private:
        ErrorCategory m_category;
        ErrorCode m_code;
        std::string m_message;
        absl::optional<ErrorContext> m_context;
        std::vector<std::unique_ptr<Error>> m_causes;

        absl::StatusCode to_absl_status_code() const
        {
            switch (m_category)
            {
                case ErrorCategory::GENERAL:
                    switch (m_code)
                    {
                        case ErrorCode::INVALID_ARGUMENT: return absl::StatusCode::kInvalidArgument;
                        case ErrorCode::NOT_IMPLEMENTED: return absl::StatusCode::kUnimplemented;
                        case ErrorCode::OPERATION_CANCELLED: return absl::StatusCode::kCancelled;
                        default: return absl::StatusCode::kInternal;
                    }
                case ErrorCategory::FILE_IO:
                    switch (m_code)
                    {
                        case ErrorCode::FILE_NOT_FOUND: return absl::StatusCode::kNotFound;
                        case ErrorCode::FILE_ACCESS_DENIED: return absl::StatusCode::kPermissionDenied;
                        default: return absl::StatusCode::kInternal;
                    }
                case ErrorCategory::VALIDATION:
                    return absl::StatusCode::kInvalidArgument;
                case ErrorCategory::RESOURCE:
                    return absl::StatusCode::kResourceExhausted;
                default:
                    return absl::StatusCode::kInternal;
            }
        }

        static const char* category_to_string(const ErrorCategory category)
        {
            switch (category)
            {
                case ErrorCategory::GENERAL: return "GENERAL";
                case ErrorCategory::FILE_IO: return "FILE_IO";
                case ErrorCategory::XML_PARSING: return "XML_PARSING";
                case ErrorCategory::DOCX_FORMAT: return "DOCX_FORMAT";
                case ErrorCategory::MEDIA_HANDLING: return "MEDIA_HANDLING";
                case ErrorCategory::ELEMENT_OPERATION: return "ELEMENT_OPERATION";
                case ErrorCategory::VALIDATION: return "VALIDATION";
                case ErrorCategory::RESOURCE: return "RESOURCE";
                default: return "UNKNOWN";
            }
        }
    };

    // ============================================================================
    // Result Types (absl::StatusOr alternative for DuckX)
    // ============================================================================

    template<typename T>
    class Result
    {
    public:
        // Success constructor
        explicit Result(T&& value)
            : m_value(std::move(value)) {}

        explicit Result(const T& value)
            : m_value(value) {}

        // Error constructor
        explicit Result(const Error& error)
            : m_error(error) {}

        explicit Result(Error&& error)
            : m_error(std::move(error)) {}

        // Implicit conversion from absl::StatusOr
        explicit Result(const absl::StatusOr<T>& status_or)
        {
            if (status_or.ok())
            {
                m_value = status_or.value();
            }
            else
            {
                m_error = Error(ErrorCategory::GENERAL, ErrorCode::UNKNOWN_ERROR,
                                status_or.status().message());
            }
        }

        // Check if result contains value
        bool ok() const { return m_value.has_value(); }
        bool has_error() const { return m_error.has_value(); }

        // Access value (will throw if error)
        const T& value() const
        {
            if (!ok())
            {
                throw std::runtime_error("Accessing value of failed Result: " + m_error->to_string());
            }
            return *m_value;
        }

        T& value()
        {
            if (!ok())
            {
                throw std::runtime_error("Accessing value of failed Result: " + m_error->to_string());
            }
            return *m_value;
        }

        // Access error
        const Error& error() const
        {
            if (!has_error())
            {
                static Error success_error;
                return success_error;
            }
            return *m_error;
        }

        // Value extraction with default
        T value_or(const T& default_value) const
        {
            return ok() ? *m_value : default_value;
        }

        // Convert to absl::StatusOr
        absl::StatusOr<T> to_status_or() const
        {
            if (ok())
            {
                return *m_value;
            }
            return m_error->to_status();
        }

        // Monadic operations
        template<typename F>
        auto and_then(F&& f) -> Result<decltype(f(std::declval<T &>()))>
        {
            if (!ok())
            {
                return m_error.value();
            }
            return f(m_value.value());
        }

        template<typename F>
        Result<T> or_else(F&& f)
        {
            if (ok())
            {
                return *this;
            }
            return f(m_error.value());
        }

    private:
        absl::optional<T> m_value;
        absl::optional<Error> m_error;
    };

    // Specialization for void
    template<>
    class Result<void>
    {
    public:
        Result() = default;

        explicit Result(const Error& error)
            : m_error(error) {}

        explicit Result(Error&& error)
            : m_error(std::move(error)) {}

        bool ok() const { return !m_error.has_value(); }
        bool has_error() const { return m_error.has_value(); }

        const Error& error() const
        {
            if (!has_error())
            {
                static Error success_error;
                return success_error;
            }
            return *m_error;
        }

        absl::Status to_status() const
        {
            return ok() ? absl::OkStatus() : m_error->to_status();
        }

    private:
        absl::optional<Error> m_error;
    };

    // ============================================================================
    // Error Factory Functions
    // ============================================================================

    namespace errors
    {
        inline Error file_not_found(const absl::string_view path, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::FILE_IO, ErrorCode::FILE_NOT_FOUND,
                absl::StrFormat("File not found: %s", path), ctx
            };
        }

        inline Error file_access_denied(const absl::string_view path, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::FILE_IO, ErrorCode::FILE_ACCESS_DENIED,
                absl::StrFormat("Access denied: %s", path), ctx
            };
        }

        inline Error xml_parse_error(const absl::string_view details, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::XML_PARSING, ErrorCode::XML_PARSE_ERROR,
                absl::StrFormat("XML parse error: %s", details), ctx
            };
        }

        inline Error docx_invalid_format(const absl::string_view details, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::DOCX_FORMAT, ErrorCode::DOCX_INVALID_FORMAT,
                absl::StrFormat("Invalid DOCX format: %s", details), ctx
            };
        }

        inline Error element_not_found(const absl::string_view element_type, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::ELEMENT_OPERATION, ErrorCode::ELEMENT_NOT_FOUND,
                absl::StrFormat("Element not found: %s", element_type), ctx
            };
        }

        inline Error validation_failed(const absl::string_view field, const absl::string_view reason,
                                       const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::VALIDATION, ErrorCode::VALIDATION_FAILED,
                absl::StrFormat("Validation failed for %s: %s", field, reason), ctx
            };
        }

        inline Error invalid_argument(const absl::string_view arg_name, const absl::string_view reason,
                                      const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::GENERAL, ErrorCode::INVALID_ARGUMENT,
                absl::StrFormat("Invalid argument '%s': %s", arg_name, reason), ctx
            };
        }

        // Style system errors
        inline Error style_not_found(const absl::string_view style_name, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::STYLE_SYSTEM, ErrorCode::STYLE_NOT_FOUND,
                absl::StrFormat("Style not found: %s", style_name), ctx
            };
        }

        inline Error style_already_exists(const absl::string_view style_name, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::STYLE_SYSTEM, ErrorCode::STYLE_ALREADY_EXISTS,
                absl::StrFormat("Style already exists: %s", style_name), ctx
            };
        }

        inline Error style_application_failed(const absl::string_view style_name, const absl::string_view reason,
                                             const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::STYLE_SYSTEM, ErrorCode::STYLE_APPLICATION_FAILED,
                absl::StrFormat("Failed to apply style '%s': %s", style_name, reason), ctx
            };
        }

        // Template system errors
        inline Error template_not_found(const absl::string_view template_path, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::TEMPLATE_SYSTEM, ErrorCode::TEMPLATE_NOT_FOUND,
                absl::StrFormat("Template not found: %s", template_path), ctx
            };
        }

        inline Error template_syntax_error(const absl::string_view details, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::TEMPLATE_SYSTEM, ErrorCode::TEMPLATE_SYNTAX_ERROR,
                absl::StrFormat("Template syntax error: %s", details), ctx
            };
        }

        inline Error template_variable_undefined(const absl::string_view var_name, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::TEMPLATE_SYSTEM, ErrorCode::TEMPLATE_VARIABLE_UNDEFINED,
                absl::StrFormat("Template variable undefined: %s", var_name), ctx
            };
        }

        // Engineering tools errors
        inline Error requirement_table_invalid(const absl::string_view reason, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::ENGINEERING_TOOLS, ErrorCode::REQUIREMENT_TABLE_INVALID,
                absl::StrFormat("Requirement table invalid: %s", reason), ctx
            };
        }

        inline Error code_block_format_unsupported(const absl::string_view language, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::ENGINEERING_TOOLS, ErrorCode::CODE_BLOCK_FORMAT_UNSUPPORTED,
                absl::StrFormat("Code block format unsupported: %s", language), ctx
            };
        }

        // Table-specific validation errors
        inline Error invalid_table_dimensions(int rows, int cols, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::VALIDATION, ErrorCode::INVALID_TABLE_DIMENSIONS,
                absl::StrFormat("Invalid table dimensions: %dx%d", rows, cols), ctx
            };
        }

        inline Error invalid_border_style(const absl::string_view style, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::VALIDATION, ErrorCode::INVALID_BORDER_STYLE,
                absl::StrFormat("Invalid border style: %s", style), ctx
            };
        }

        inline Error invalid_width_value(double width, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::VALIDATION, ErrorCode::INVALID_WIDTH_VALUE,
                absl::StrFormat("Invalid width value: %f", width), ctx
            };
        }

        inline Error invalid_height_value(double height, const ErrorContext& ctx = {})
        {
            return {
                ErrorCategory::VALIDATION, ErrorCode::INVALID_HEIGHT_VALUE,
                absl::StrFormat("Invalid height value: %f", height), ctx
            };
        }
    } // namespace errors

    // ============================================================================
    // Error Handler and Callback System
    // ============================================================================

    class ErrorHandler
    {
    public:
        using ErrorCallback = std::function<void(const Error&)>;
        using ErrorFilter = std::function<bool(const Error&)>;

        // Register error callbacks
        void on_error(const ErrorCallback& callback)
        {
            m_global_callbacks.push_back(callback);
        }

        void on_error(const ErrorCategory category, const ErrorCallback& callback)
        {
            m_category_callbacks[category].push_back(callback);
        }

        void on_error(const ErrorCode code, const ErrorCallback& callback)
        {
            m_code_callbacks[code].push_back(callback);
        }

        // Handle error
        void handle(const Error& error)
        {
            // Call global callbacks
            for (const auto& callback: m_global_callbacks)
            {
                callback(error);
            }

            // Call category-specific callbacks
            const auto cat_it = m_category_callbacks.find(error.category());
            if (cat_it != m_category_callbacks.end())
            {
                for (const auto& callback: cat_it->second)
                {
                    callback(error);
                }
            }

            // Call code-specific callbacks
            const auto code_it = m_code_callbacks.find(error.code());
            if (code_it != m_code_callbacks.end())
            {
                for (const auto& callback: code_it->second)
                {
                    callback(error);
                }
            }
        }

        // Get singleton instance
        static ErrorHandler& instance()
        {
            static ErrorHandler handler;
            return handler;
        }

    private:
        std::vector<ErrorCallback> m_global_callbacks;
        std::map<ErrorCategory, std::vector<ErrorCallback>> m_category_callbacks;
        std::map<ErrorCode, std::vector<ErrorCallback>> m_code_callbacks;
    };

    // ============================================================================
    // Macros for convenient error handling
    // ============================================================================

#define DUCKX_ERROR_CONTEXT() \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__)

#define DUCKX_ERROR_CONTEXT_OP(operation) \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__, operation)

// Enhanced context macros for specific scenarios
#define DUCKX_ERROR_CONTEXT_TABLE(operation, row, col) \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__, operation).with_table_position(row, col)

#define DUCKX_ERROR_CONTEXT_STYLE(operation, style_name) \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__, operation).with_style_name(style_name)

#define DUCKX_ERROR_CONTEXT_TEMPLATE(operation, template_name) \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__, operation).with_template_name(template_name)

#define DUCKX_ERROR_CONTEXT_ELEMENT(operation, element_type) \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__, operation).with_element_type(element_type)

#define DUCKX_ERROR_CONTEXT_VALIDATION(operation, rule_name) \
    duckx::ErrorContext(__FILE__, __FUNCTION__, __LINE__, operation).with_validation_rule(rule_name)

#define DUCKX_RETURN_IF_ERROR(result) \
    do { \
        if (!result.ok()) { \
            return result.error(); \
        } \
    } while(0)

#define DUCKX_ASSIGN_OR_RETURN(lhs, rhs) \
    do { \
        auto _result = (rhs); \
        if (!_result.ok()) { \
            return _result.error(); \
        } \
        lhs = std::move(_result.value()); \
    } while(0)

#define DUCKX_HANDLE_ERROR(error) \
    duckx::ErrorHandler::instance().handle(error)
} // namespace duckx
