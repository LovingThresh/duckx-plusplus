/*!
 * @brief
 *
 * @author LiuYe
 * @date 2025.06.29
 * @copyright (c) 2013-2025 Honghu Yuntu Corporation
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

namespace duckx
{
    // ============================================================================
    // Error Categories and Codes
    // ============================================================================

    enum class ErrorCategory
    {
        GENERAL = 0,
        FILE_IO = 1,
        XML_PARSING = 2,
        DOCX_FORMAT = 3,
        MEDIA_HANDLING = 4,
        ELEMENT_OPERATION = 5,
        VALIDATION = 6,
        RESOURCE = 7
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

        // Resource errors (700-799)
        MEMORY_ALLOCATION_FAILED = 700,
        RESOURCE_LIMIT_EXCEEDED = 701,
        DEPENDENCY_NOT_AVAILABLE = 702
    };

    // ============================================================================
    // Error Context and Details
    // ============================================================================

    struct ErrorContext
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

    class Error
    {
    public:
        Error()
            : m_category(ErrorCategory::GENERAL), m_code(ErrorCode::SUCCESS) {}

        Error(const ErrorCategory category, const ErrorCode code, const absl::string_view message)
            : m_category(category), m_code(code), m_message(message.data(), message.size()) {}

        Error(const ErrorCategory category, const ErrorCode code, const absl::string_view message,
              const ErrorContext& context)
            : m_category(category), m_code(code), m_message(message.data(), message.size()), m_context(context) {}

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
