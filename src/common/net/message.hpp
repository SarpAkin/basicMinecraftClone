#pragma once

#include <assert.h>
#include <array>
#include <cstdint>
#include <experimental/type_traits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT_ __FILE__ ":" TOSTRING(__LINE__)

#define CHECK_TYPE(T)                                                                                                  \
    static_assert(std::is_trivially_copyable<T>::value, #T AT_);                                                       \
    static_assert(std::is_standard_layout<T>::value, #T AT_);

class Message
{
private:
    int index = 0;
    std::vector<char> data_;

private:
public:
    Message() = default;

    // ereases popped parts which were already going to be erased
    inline void trim()
    {
        if (index)
        {
            data_.erase(data_.begin(), data_.begin() + index);
            index = 0;
        }
    }

    inline size_t size() const
    {
        return data_.size() - index;
    }

    inline std::vector<char>& data()
    {
        trim();
        return data_;
    }

    inline const std::vector<char>& data() const
    {
        return data_;
    }

    class Test
    {
    public:
        void foo();
    };

    template <typename T>
    void push_back(const T& item)
    {
        constexpr bool hasFunc = requires(T & t, Message & m)
        {
            t.Serialize(m);
            t.Deserialize(m);
        };
        if constexpr (hasFunc)
        {
            item.Serialize(*this);
        }
        else
        {
            CHECK_TYPE(T);
            data_.insert(data_.end(), (char*)&item, (char*)&item + sizeof(T));
        }
    }

    template <typename T>
    inline void push_back(T&& item)
    {
        const auto& ref = item;
        push_back(ref);
    }

    template <typename T>
    inline void push_back(const std::unique_ptr<T>& item)
    {
        if (item)
        {
            push_back(uint8_t(1));
            push_back(*item);
        }
        else
        {
            push_back(uint8_t(0));
        }
    }

    template <typename T>
    inline void pop_front(std::unique_ptr<T>& item)
    {
        if (pop_front<uint8_t>())
        {
            if (item)
            {
                pop_front(*item);
            }
            else
            {
                constexpr bool has_def_constructor = requires(std::unique_ptr<T> ptr)
                {
                    ptr = std::make_unique<T>();
                };
                if constexpr (has_def_constructor)
                {
                    item = std::make_unique<T>();
                    pop_front(*item);
                }
                else 
                {
                    //give assertion if T can't be copy constructed.
                    assert(0);
                }
            }
        }
    }

    inline void push_back(const std::string& item)
    {
        push_back((uint32_t)item.size());
        data_.insert(data_.end(), item.begin(), item.end());
    }

    template <typename T, size_t L>
    void push_back(const std::array<T, L>& item);

    template <typename T>
    void push_back(const std::vector<T>& item);

    template <typename T>
    T pop_front()
    {

        constexpr bool hasFunc = requires(T & t, Message & m)
        {
            t.Serialize(m);
            t.Deserialize(m);
        };
        if constexpr (hasFunc)
        {
            T tmp;
            tmp.Deserialize(*this);
            return tmp;
        }
        else
        {
            CHECK_TYPE(T);
            T tmp = *(T*)(data_.data() + index);
            index += sizeof(T);
            return tmp;
        }
        // data_.erase(data_.begin(), data_.begin() + sizeof(T));
    }

    template <typename T>
    void pop_front(T& item)
    {
        constexpr bool hasFunc = requires(T & t, Message & m)
        {
            t.Serialize(m);
            t.Deserialize(m);
        };
        if constexpr (hasFunc)
        {
            item.Deserialize(*this);
        }
        else
        {
            CHECK_TYPE(T);
            item = *(T*)(data_.data() + index);
            index += sizeof(T);
        }
        // data_.erase(data_.begin(), data_.begin() + sizeof(T));
    }

    inline void pop_front(std::string& item)
    {
        uint32_t size_ = pop_front<uint32_t>();
        item = std::string(data_.data() + index, data_.data() + index + size_);
        index += size_;
    }

    template <typename T, size_t L>
    void pop_front(std::array<T, L>& item);

    template <typename T>
    void pop_front(std::vector<T>& item);
};

template <typename T, size_t L>
void Message::push_back(const std::array<T, L>& item)
{
    push_back((uint32_t)item.size());
    for (const T& item_ : item)
        push_back(item_);
}

template <typename T>
void Message::push_back(const std::vector<T>& item)
{
    push_back((uint32_t)item.size());
    for (const T& item_ : item)
        push_back(item_);
}

template <typename T, size_t L>
void Message::pop_front(std::array<T, L>& item)
{
    uint32_t size_ = pop_front<uint32_t>();
    for (auto& item_ : item)
        pop_front(item_);
}

template <typename T>
void Message::pop_front(std::vector<T>& item)
{
    uint32_t size_ = pop_front<uint32_t>();
    item = std::vector<T>(size_);
    for (auto& item_ : item)
        pop_front(item_);
}

#undef STRINGIFY
#undef TOSTRING
#undef AT_

#undef CHECK_TYPE
