/*!
 * @file duckxiterator.hpp
 * @brief Iterator and range utilities for DOCX document elements
 * 
 * Provides type-safe iterators and ranges for traversing document elements
 * like paragraphs, tables, runs, and cells with STL-compatible interfaces.
 */
#pragma once
#include <iterator>

#include "absl/meta/type_traits.h"

namespace pugi
{
    class xml_node;
}

namespace duckx
{
    /*! @brief SFINAE helper to detect get_node() method */
    template<typename T>
    class has_get_node_method
    {
        template<typename U>
        static auto test(int) -> decltype(std::declval<U>().get_node(), std::true_type {}) { return {}; }

        template<typename>
        static std::false_type test(...) { return {}; }

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    class has_advance_method
    {
        template<typename U>
        static auto test(int) -> decltype(std::declval<U>().advance(), std::true_type {}) { return {}; }

        template<typename>
        static std::false_type test(...) { return {}; }

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    class has_valid_get_node_return
    {
        template<typename U>
        static auto test(int) -> std::is_same<decltype(std::declval<U>().get_node()), pugi::xml_node> { return {}; }

        template<typename>
        static std::false_type test(...) { return {}; }

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    class has_valid_advance_return
    {
        template<typename U>
        static auto test(int) -> std::is_same<decltype(std::declval<U>().advance()), U &> { return {}; }

        template<typename>
        static std::false_type test(...) { return {}; }

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    class has_valid_try_advance_return
    {
        template<typename U>
        static auto test(int) -> std::is_same<decltype(std::declval<U>().try_advance()), bool> { return {}; }

        template<typename>
        static std::false_type test(...) { return {}; }

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    /*! @brief Type trait to check if T is a valid DOCX element */
    template<typename T>
    struct is_docx_element
    {
        static constexpr bool value =
                has_get_node_method<T>::value &&
                has_advance_method<T>::value &&
                has_valid_get_node_return<T>::value &&
                has_valid_advance_return<T>::value &&
                has_valid_try_advance_return<T>::value;
    };

    template<typename T>
    using enable_if_docx_element_t = absl::enable_if_t<is_docx_element<T>::value, int>;

    /*!
     * @brief STL-compatible iterator for DOCX document elements
     * 
     * Provides forward iteration over document elements like paragraphs,
     * runs, tables, and cells with standard iterator semantics.
     */
    template<class T, enable_if_docx_element_t<T>  = 0>
    class ElementIterator
    {
        static_assert(is_docx_element<T>::value,
                      "ElementIterator requires a type with get_node() -> pugi::xml_node and advance() -> bool methods");

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        explicit ElementIterator(T element)
            : m_current_element(std::move(element)), m_is_end(false)
        {
            if (!m_current_element.get_node())
            {
                m_is_end = true;
            }
        }

        ElementIterator()
            : m_is_end(true) {}

        reference operator*()
        {
            return m_current_element;
        }

        pointer operator->()
        {
            return &m_current_element;
        }

        ElementIterator& operator++()
        {
            if (!m_current_element.try_advance() || !m_current_element.get_node())
            {
                m_is_end = true;
            }
            return *this;
        }

        friend bool operator==(const ElementIterator& a, const ElementIterator& b)
        {
            if (a.m_is_end && b.m_is_end)
            {
                return true;
            }
            if (a.m_is_end || b.m_is_end)
            {
                return false;
            }
            return a.m_current_element.get_node() == b.m_current_element.get_node();
        }

        friend bool operator!=(const ElementIterator& a, const ElementIterator& b)
        {
            return !(a == b);
        }

    private:
        T m_current_element;
        bool m_is_end;
    };

    /*!
     * @brief Range wrapper for DOCX document elements
     * 
     * Provides STL-compatible range interface for document elements,
     * enabling range-based for loops and STL algorithms.
     */
    template<class T, enable_if_docx_element_t<T>  = 0>
    class ElementRange
    {
        static_assert(is_docx_element<T>::value,
                      "ElementRange requires a type with get_node() -> pugi::xml_node and advance() -> bool methods");

    public:
        explicit ElementRange(T element_state)
            : m_element_state(element_state) {}

        ElementIterator<T> begin()
        {
            return ElementIterator<T>(m_element_state);
        }

        ElementIterator<T> end()
        {
            return ElementIterator<T>();
        }

        ElementIterator<T> begin() const
        {
            return ElementIterator<T>(m_element_state);
        }

        ElementIterator<T> end() const
        {
            return ElementIterator<T>();
        }

        ElementIterator<T> cbegin() const
        {
            return ElementIterator<T>(m_element_state);
        }

        ElementIterator<T> cend() const
        {
            return ElementIterator<T>();
        }

        /*! @brief Check if the range is empty */
        bool empty() const
        {
            return !m_element_state.get_node();
        }

        /*! @brief Get the first element in the range */
        T first() const
        {
            return m_element_state;
        }

        /*! @brief Calculate the size of the range */
        size_t size() const
        {
            size_t count = 0;
            T current = m_element_state;

            if (current.get_node())
            {
                count = 1;
                while (current.try_advance())
                {
                    count++;
                }
            }

            return count;
        }

    private:
        T m_element_state;
    };

    /*! @brief Factory function to create element ranges */   
    template<typename T>
    auto make_element_range(T element)
        -> absl::enable_if_t<is_docx_element<T>::value, ElementRange<T>>
    {
        return ElementRange<T>(element);
    }

    template<typename T>
    auto make_element_range(T element)
        -> absl::enable_if_t<!is_docx_element<T>::value, void>
    {
        static_assert(has_get_node_method<T>::value,
                      "Type must have a get_node() method returning pugi::xml_node");
        static_assert(has_advance_method<T>::value,
                      "Type must have an advance() method returning bool");
        static_assert(has_valid_get_node_return<T>::value,
                      "get_node() must return pugi::xml_node");
        static_assert(has_valid_advance_return<T>::value,
                      "advance() must return T&");
        static_assert(has_valid_try_advance_return<T>::value,
                      "try_advance() must return bool");
    }

    namespace detail
    {
        template<typename T, typename Base>
        struct is_derived_from_docx_element
        {
            static constexpr bool value =
                    std::is_base_of<Base, T>::value && is_docx_element<T>::value;
        };

        template<typename T>
        class has_runs_method
        {
            template<typename U>
            static auto test(int) -> decltype(std::declval<U>().runs(), std::true_type {}) { return {}; }

            template<typename>
            static std::false_type test(...) { return {}; }

        public:
            static constexpr bool value = decltype(test<T>(0))::value;
        };

        template<typename T>
        class has_cells_method
        {
            template<typename U>
            static auto test(int) -> decltype(std::declval<U>().cells(), std::true_type {}) { return {}; }

            template<typename>
            static std::false_type test(...) { return {}; }

        public:
            static constexpr bool value = decltype(test<T>(0))::value;
        };
    }

    template<typename T>
    using ParagraphLikeIterator = absl::enable_if_t<
        is_docx_element<T>::value && detail::has_runs_method<T>::value,
        ElementIterator<T>>;

    template<typename T>
    using TableRowLikeIterator = absl::enable_if_t<
        is_docx_element<T>::value && detail::has_cells_method<T>::value,
        ElementIterator<T>>;
} // namespace duckx
