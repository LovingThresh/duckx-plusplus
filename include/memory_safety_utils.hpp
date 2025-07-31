#pragma once

#include <memory>
#include <type_traits>
#include <string>
#include <functional>
#include <mutex>
#ifdef _WIN32
#include <windows.h>
#endif

// Memory safety utilities for cross-platform compatibility

namespace MemorySafety
{

    // Safe pointer validation
    template<typename T>
    bool isValidPointer(T* ptr)
    {
        if (!ptr)
            return false;

#ifdef _MSC_VER
    // Windows-specific pointer validation
    __try {
        // Try to read from the pointer
        volatile auto temp = *ptr;
        (void)temp;
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
#else
        // On other platforms, non-null is considered valid
        return true;
#endif
    }

    // Safe string validation
    inline bool isValidString(const char* str, size_t maxLen = 1024)
    {
        if (!str)
            return false;

#ifdef _MSC_VER
    return !IsBadStringPtrA(str, maxLen);
#else
        // Basic validation on other platforms
        for (size_t i = 0; i < maxLen; ++i)
        {
            if (str[i] == '\0')
                return true;
        }
        return false;
#endif
    }

    // Safe member function call wrapper
    template<typename Obj, typename Ret, typename... Args>
    class SafeMemberFunction
    {
    private:
        std::weak_ptr<Obj> m_object;

        Ret (Obj::*m_func)(Args...);

        Ret m_default;

    public:
        SafeMemberFunction(std::shared_ptr<Obj> obj, Ret (Obj::*func)(Args...), Ret defaultVal)
            : m_object(obj), m_func(func), m_default(defaultVal)
        {}

        Ret operator()(Args... args)
        {
            if (auto obj = m_object.lock())
            {
                try
                {
                    return (obj.get()->*m_func)(args...);
                }
                catch (...)
                {
                    return m_default;
                }
            }
            return m_default;
        }
    };

    // Memory barrier wrapper
    inline void memoryBarrier()
    {
#ifdef _MSC_VER
    _ReadWriteBarrier();
    MemoryBarrier();
#elif defined(__GNUC__)
        __sync_synchronize();
#else
    std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
    }

    // Safe dynamic cast with validation
    template<typename To, typename From>
    std::shared_ptr<To> safeDynamicCast(const std::shared_ptr<From>& ptr)
    {
        if (!ptr)
            return nullptr;

        try
        {
            // Ensure memory coherence before cast
            memoryBarrier();

            auto result = std::dynamic_pointer_cast<To>(ptr);

            // Validate the result
            if (result && !isValidPointer(result.get()))
            {
                return nullptr;
            }

            return result;
        }
        catch (...)
        {
            return nullptr;
        }
    }

    // Thread-safe lazy initialization
    template<typename T>
    class LazyInit
    {
    private:
        mutable std::shared_ptr<T> m_instance;
        mutable std::mutex m_mutex;
        std::function<std::shared_ptr<T>()> m_factory;

    public:
        explicit LazyInit(std::function<std::shared_ptr<T>()> factory)
            : m_factory(factory)
        {}

        std::shared_ptr<T> get() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_instance)
            {
                m_instance = m_factory();
            }
            return m_instance;
        }

        void reset()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_instance.reset();
        }
    };

    // Safe string operations
    class SafeString
    {
    private:
        std::string m_data;
        mutable std::mutex m_mutex;

    public:
        SafeString() = default;

        explicit SafeString(const std::string& str) : m_data(str)
        {}

        std::string get() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_data;
        }

        void set(const std::string& str)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data = str;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_data.empty();
        }
    };

    // RAII memory protection for Windows
#ifdef _WIN32
class MemoryProtectionGuard {
private:
    void* m_address;
    SIZE_T m_size;
    DWORD m_oldProtect;
    bool m_protected;

public:
    MemoryProtectionGuard(void* address, SIZE_T size, DWORD newProtect)
        : m_address(address), m_size(size), m_protected(false) {
        if (VirtualProtect(m_address, m_size, newProtect, &m_oldProtect)) {
            m_protected = true;
        }
    }

    ~MemoryProtectionGuard() {
        if (m_protected) {
            DWORD temp;
            VirtualProtect(m_address, m_size, m_oldProtect, &temp);
        }
    }

    bool isProtected() const { return m_protected; }
};
#endif

    // Exception-safe resource manager
    template<typename T, typename Deleter = std::default_delete<T>>
    class SafeResource
    {
    private:
        std::unique_ptr<T, Deleter> m_resource;
        std::function<void(T*)> m_errorHandler;

    public:
        SafeResource(T* resource = nullptr,
                     Deleter deleter = Deleter(),
                     std::function<void(T*)> errorHandler = nullptr)
            : m_resource(resource, deleter), m_errorHandler(errorHandler)
        {}

        T* get() const
        {
            return m_resource.get();
        }

        T* operator->() const
        {
            return m_resource.get();
        }

        T& operator*() const
        {
            return *m_resource;
        }

        void reset(T* resource = nullptr)
        {
            try
            {
                m_resource.reset(resource);
            }
            catch (...)
            {
                if (m_errorHandler && resource)
                {
                    m_errorHandler(resource);
                }
                throw;
            }
        }

        T* release()
        {
            return m_resource.release();
        }
    };

} // namespace MemorySafety
