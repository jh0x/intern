#pragma once

// The MIT License (MIT)
//
// Copyright (c) 2019 jh0x
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iterator>

namespace intern {
namespace details {

template<typename T, typename Traits>
struct string_common
{
    using traits_type            = Traits;
    using value_type             = char;
    using pointer                = const char*;
    using const_pointer          = const char*;
    using reference              = const char&;
    using const_reference        = const char&;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using size_type              = typename traits_type::size_type;

protected:
    string_common() = default;
    string_common(const string_common&) = default;
    string_common& operator=(const string_common&) = default;

public:
    void swap(string_common& o) noexcept
    {
        *static_cast<T*>(this)->swap(*static_cast<T*>(&o));
    }

    const_pointer data() const noexcept
    {
        return static_cast<const T*>(this)->data();
    }

    size_type size() const noexcept
    {
        return static_cast<const T*>(this)->size();
    }

    size_type length() const noexcept { return size(); }
    bool empty() const noexcept { return !size(); }

    const_pointer c_str() const noexcept { return data(); }
    const_reference operator[](size_type idx) const noexcept
    {
        return data()[idx];
    }

    const_reference front() const noexcept { return data()[0]; }
    const_reference back() const noexcept { return data()[size() - 1]; }

    const_iterator begin() const noexcept { return data(); }
    const_iterator end() const noexcept { return data() + size(); }

    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator{end()};
    }
    const_reverse_iterator rend() const {
        return const_reverse_iterator{begin()};
    }

    const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    const_reverse_iterator crend() const noexcept { return rend(); }

    template<typename U>
    int compare(const string_common<U, Traits>& o) const noexcept
    {
        if(auto result = Traits::cmp(data(), o.data(),
                    std::min(size(), o.size())))
        {
            return result;
        }
        return size() == o.size() ? 0 : size() < o.size() ? -1 : 1;
    }
};

}

template<size_t N, typename T, typename Traits>
bool operator==(
        const details::string_common<T, Traits>& a,
        const char (&b)[N]) noexcept
{
    return a.size() == (N - 1) && !Traits::cmp(a.data(), b, N - 1);
}
template<size_t N, typename T, typename Traits>
bool operator!=(
        const details::string_common<T, Traits>& a,
        const char (&b)[N]) noexcept
{
    return ! (a == b);
}

template<typename T, typename U, typename Traits>
bool operator==(
        const details::string_common<T, Traits>& a,
        const details::string_common<U, Traits>& b) noexcept
{
    return a.size() == b.size() && !Traits::cmp(a.data(), b.data(), a.size());
}
template<typename T, typename U, typename Traits>
bool operator!=(
        const details::string_common<T, Traits>& a,
        const details::string_common<U, Traits>& b) noexcept
{
    return !(a == b);
}
template<typename T, typename U, typename Traits>
bool operator<(
        const details::string_common<T, Traits>& a,
        const details::string_common<U, Traits>& b) noexcept
{
    return a.compare(b) < 0;
}
template<typename T, typename U, typename Traits>
bool operator<=(
        const details::string_common<T, Traits>& a,
        const details::string_common<U, Traits>& b) noexcept
{
    return a.compare(b) <= 0;
}
template<typename T, typename U, typename Traits>
bool operator>=(
        const details::string_common<T, Traits>& a,
        const details::string_common<U, Traits>& b) noexcept
{
    return a.compare(b) >= 0;
}
template<typename T, typename U, typename Traits>
bool operator>(
        const details::string_common<T, Traits>& a,
        const details::string_common<U, Traits>& b) noexcept
{
    return a.compare(b) > 0;
}

}

