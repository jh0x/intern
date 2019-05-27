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
class string_sso_v2
    : public details::string_common<string_sso_v2<S, Traits>, Traits>
{
public:
    using MyT = string_sso_v2<S, Traits>;
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
    constexpr static auto _max_size = S - sizeof(char*) - sizeof(size_type);
    constexpr static auto sso_size = _max_size - 1;
    static_assert(S >= _min_sso_size, "size too small");
    static_assert(S % 8 == 0, "size should be a multiple of 8");

    constexpr string_sso_v2(const string_sso_v2& o) noexcept
        : BaseT{o}
        , _len{o._len}
    {
        if(INTERN__LIKELY(o.small()))
        {
            Traits::copy(_data, o._data, _max_size);
            _ptr = _data;
        }
        else
        {
            _ptr = o._ptr;
        }
    }

    constexpr string_sso_v2& operator=(const string_sso_v2& o)
    {
        if(INTERN__UNLIKELY(this == &o)) return *this;
        _len = o._len;
        if(INTERN__LIKELY(o.small()))
        {
            Traits::copy(_data, o._data, _max_size);
            _ptr = _data;
        }
        else
        {
            _ptr = o._ptr;
        }
        return *this;
    }

    constexpr void swap(string_sso_v2& o) noexcept
    {
        using std::swap;
        if(small() & o.small())
        {
            // nothing
        }
        else if(small())
        {
            _ptr = o._ptr;
            o._ptr = o._data;
        }
        else if(o.small())
        {
            o._ptr = _ptr;
            _ptr = _data;
        }
        else
        {
            swap(_ptr, o._ptr);
        }
        swap(_len, o._len);
        swap(_data, o._data);
    }

    constexpr bool small() const noexcept {
        return _ptr == _data;
    }

    constexpr size_type size() const noexcept {
        return _len;
    }

    constexpr const_pointer data() const noexcept
    {
        return _ptr;
    }

    template<std::size_t N>
    constexpr bool operator==(const string_sso_v2<N, Traits>& o) const
    {
        return size() == o.size() && !Traits::cmp(data(), o.data(), size());
    }

private:
    constexpr string_sso_v2(const char* s, size_type sz)
    {
        if(INTERN__LIKELY(sz < _max_size))
        {
            _ptr = _data;
            Traits::copy(_data, s, sz);
            _data[sz] = '\0';
            _len = sz;
        }
        else
        {
            _ptr = s;
            _len = sz;
        }
    }

    const char* _ptr;
    size_type _len;
    char _data[_max_size];
};

}

