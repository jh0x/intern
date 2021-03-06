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

#include <intern/details/string_common.hpp>
#include <intern/details/metadata.hpp>

namespace intern
{

template<typename Traits>
class string_far
    : public details::string_common<string_far<Traits>, Traits>
{
public:
    using MyT = string_far<Traits>;
    using BaseT = details::string_common<MyT, Traits>;
    using typename BaseT::value_type;
    using typename BaseT::pointer;
    using typename BaseT::const_pointer;
    using typename BaseT::reference;
    using typename BaseT::const_reference;
    using typename BaseT::iterator;
    using typename BaseT::const_iterator;
    using typename BaseT::reverse_iterator;
    using typename BaseT::const_reverse_iterator;
    using typename BaseT::size_type;
    template<typename, typename> friend class interner;
    constexpr static auto sso_size = 0;

    constexpr explicit string_far(const char* data) : _data{data} {}
    constexpr string_far(const string_far&) = default;
    constexpr string_far& operator=(const string_far&) = default;

    constexpr void swap(string_far& o) noexcept
    {
        using std::swap;
        swap(_data, o._data);
    }

    constexpr size_type size() const noexcept { return _meta()._len; }
    constexpr const_pointer data() const noexcept { return _data; }
    constexpr bool small() const noexcept { return false; }

private:
    using metadata = details::metadata<Traits>;
    constexpr const metadata& _meta() const noexcept
    {
        return *reinterpret_cast<const metadata*>(
                &this->front() - sizeof(metadata));
    }
private:
    const char* _data;
};

template<typename Traits>
inline constexpr bool operator==(
        string_far<Traits> a,
        string_far<Traits> b) noexcept
{
    if(Traits::string_far_use_ptr_equality)
    {
        return a.data() == b.data();
    }
    else
    {
        return a.size() == b.size() &&
            !Traits::cmp(a.data(), b.data(), a.size());
    }
}

template<typename Traits>
inline constexpr bool operator!=(
        string_far<Traits> a,
        string_far<Traits> b) noexcept
{
    return !(a == b);
}

}
