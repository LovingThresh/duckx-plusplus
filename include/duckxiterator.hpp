/*
 * Under MIT license
 * Author: Cihan SARI (@CihanSari)
 * DuckX is a free library to work wirh docx files.
 */
#pragma once
#include <iterator>

namespace pugi
{
    class xml_node;
}

namespace duckx
{
    template<class T>
    class ElementIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit ElementIterator(T element) : m_current_element(std::move(element)), m_is_end(false)
        {
            // 如果初始节点无效，则这是一个 "end" 迭代器
            if (!m_current_element.getNode())
            {
                m_is_end = true;
            }
        }

        ElementIterator() : m_is_end(true)
        {}

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
            m_current_element.next();
            // 如果 next() 使得节点无效，则迭代器到达末尾
            if (!m_current_element.getNode())
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
            return a.m_current_element.getNode() == b.m_current_element.getNode();
        };

        friend bool operator!=(const ElementIterator& a, const ElementIterator& b)
        {
            return !(a == b);
        };

    private:
        T m_current_element;
        bool m_is_end;
    };

    template<class T>
    class ElementRange
    {
    public:
        explicit ElementRange(T element_state) : m_element_state(element_state)
        {}

        ElementIterator<T> begin()
        {
            return ElementIterator<T>(m_element_state);
        }

        ElementIterator<T> end()
        {
            return ElementIterator<T>();
        }

    private:
        T m_element_state;
    };
} // namespace duckx
