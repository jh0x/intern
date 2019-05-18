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
#include <intern/string_far.hpp>

namespace intern {

template<typename Traits>
class string_sso_tiny
    : public details::string_common<
            string_sso_tiny<Traits>, Traits>
{
public:
    using MyT = string_sso_tiny<Traits>;
    using BaseT = details::string_common<MyT, Traits>;
    using FarT = string_far<Traits>;
    static_assert(sizeof(string_far<Traits>) == sizeof(char*));
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

    constexpr static auto raw_size = sizeof(char*);
    constexpr static auto sso_size = raw_size - 1;

    constexpr string_sso_tiny(const string_sso_tiny& o) = default;
    constexpr string_sso_tiny& operator=(const string_sso_tiny& o) = default;

    constexpr void swap(string_sso_tiny& o) noexcept
    {
        using std::swap;
        swap(_raw, o._raw);
    }

    constexpr bool small() const noexcept
    {
        return (_raw[sso_size] & 0x1) == 0;
    }

    constexpr size_type size() const noexcept
    {
        return small()
            ? sso_size - (_raw[sso_size] >> 1)
            : far().size();
    }

    constexpr const_pointer data() const noexcept
    {
        return small()
            ? _raw
            : far().data();
    }

    template<std::size_t N>
    constexpr bool operator==(const string_sso_tiny<Traits>& o) const
    {
        return size() == o.size() && !Traits::cmp(data(), o.data(), size());
    }

private:
    constexpr string_sso_tiny(const char* s, size_type sz)
    {
        assert(sz <= sso_size);
        Traits::copy(_raw, s, sz);
        _raw[sz] = '\0';
        _raw[sso_size] = (sso_size - sz) << 1;
    }

    constexpr string_sso_tiny(FarT far)
    {
        _ptr = far.data();
        _raw[sso_size] |= 0x1;
    }

    constexpr FarT far() const noexcept
    {
        union
        {
            const char* ptr;
            char b[raw_size];
        } tmp = {_ptr};
        tmp.ptr = _ptr;
        tmp.b[sso_size] &= ~(1u);
        return FarT(tmp.ptr);
    }

    union
    {
        const char* _ptr;
        char _raw[raw_size];
    };
};

}
