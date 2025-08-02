/*!
 * @file duckx_pimpl.hpp
 * @brief Pimpl (Private Implementation) utilities for DLL-safe interfaces
 * 
 * Provides macros and utilities to implement the Pimpl idiom for hiding
 * STL containers and other implementation details from DLL interfaces.
 * This completely eliminates C4251 warnings by moving all STL usage
 * to the implementation files.
 * 
 * @date 2025.08
 */
#pragma once

#include <memory>
#include "duckx_export.h"
#include "duckx_dll_config.hpp"

namespace duckx {

/*!
 * @brief Base class for Pimpl implementations
 * 
 * Provides a standard interface for private implementation classes.
 * This allows complete hiding of STL containers from public headers.
 */
template<typename T>
class PimplBase {
public:
    virtual ~PimplBase() = default;
};

/*!
 * @brief Helper macro to declare Pimpl pattern in class
 * 
 * Usage in class declaration:
 * @code
 * class DUCKX_API MyClass {
 *     DUCKX_DECLARE_PIMPL(MyClass)
 * public:
 *     MyClass();
 *     ~MyClass();
 *     // ... public interface ...
 * };
 * @endcode
 */
#define DUCKX_DECLARE_PIMPL(ClassName) \
private: \
    class Impl; \
    std::unique_ptr<Impl> pImpl;

/*!
 * @brief Helper macro for Pimpl implementation file
 * 
 * Usage in implementation file:
 * @code
 * DUCKX_IMPL_PIMPL(MyClass) {
 *     std::string m_path;
 *     std::map<std::string, std::string> m_data;
 *     // ... other private members ...
 * };
 * @endcode
 */
#define DUCKX_IMPL_PIMPL(ClassName) \
class ClassName::Impl : public duckx::PimplBase<ClassName::Impl>

} // namespace duckx