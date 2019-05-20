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

namespace intern {

template<std::size_t S, typename Traits>
class string_sso_v1
    : public details::string_common<
            string_sso_v1<S, Traits>, Traits>
{
public:
    using MyT = string_sso_v1<S, Traits>;
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

    constexpr static auto _min_sso_size = 16;
    static_assert(S >= _min_sso_size, "size too small");
    static_assert(S < 128, "size too big");
    static_assert(S % 8 == 0, "size problem - should be multiple of 8");
    constexpr static auto raw_size = S;
    constexpr static auto sso_size = S - 1;

    constexpr string_sso_v1(const string_sso_v1& o) = default;
    constexpr string_sso_v1& operator=(const string_sso_v1& o) = default;

    constexpr void swap(string_sso_v1& o) noexcept
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
        return small() ? _s.size() : _b.size();
    }

    constexpr const_pointer data() const noexcept
    {
        return small() ? _s._data : _b._data;
    }

    template<std::size_t N>
    constexpr bool operator==(const string_sso_v1<N, Traits>& o) const
    {
        return size() == o.size() && !Traits::cmp(data(), o.data(), size());
    }

private:
    constexpr string_sso_v1(const char* s, size_type sz)
    {
        if(INTERN__LIKELY(sz <= sso_size))
        {
            Traits::copy(_raw, s, sz);
            _raw[sz] = '\0';
            _s.size(sz);
        } else {
            _b._data = s;
            _b.size(sz);
            _raw[sso_size] |= 0x1;
        }
    }

    struct Small
    {
        constexpr size_type size() const noexcept
        {
            return sso_size - (_sz >> 1);
        }
        constexpr void size(size_type sz) noexcept
        {
            _sz = (sso_size - sz) << 1;
        }

        char _data[sso_size];
        std::uint8_t _sz;
    };
    struct Big
    {
        union Tail
        {
            std::size_t _sz;
            char _raw[sizeof(std::size_t)];
        };
        constexpr size_type size() const noexcept
        {
            Tail tmp = t;
            tmp._raw[sizeof(std::size_t) - 1] &= ~(1u);
            return tmp._sz;
        }
        constexpr void size(size_type sz) noexcept
        {
            t._sz = sz;
            t._raw[sizeof(std::size_t) - 1] |= 0x1;
        }

        const char* _data;
        char _pad[S - sizeof(char*) - sizeof(std::size_t)];
        Tail t;
    };
    static_assert(sizeof(Small) == sizeof(Big), "Sizing problem");
    static_assert(sizeof(Small) == raw_size, "Sizing problem");

    union
    {
        Small _s;
        Big _b;
        char _raw[raw_size];
    };
};

}
